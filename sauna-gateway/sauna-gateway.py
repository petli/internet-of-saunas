#!/bin/env python3

import time
import sys
from statistics import mean
import botocore
import boto3
from RF24 import *

GATEWAY_ADDRESS = b'IoSGW'
CE_PIN = 25

RESTART_MINUTES = 5


class SaunaGateway:
    def __init__(self):
        self._cloudwatch = boto3.client('cloudwatch')

        self._radio = self.open_radio()

        self._temp_samples = []
        self._humidity_samples = []
        self._last_message_id = None
        self._can_blink_led = self._set_led(1)


    def open_radio(self):
        # Use /dev/spidev0.0
        radio = RF24(CE_PIN, 0)

        radio.begin()

        radio.payloadSize = 3
        radio.setAutoAck(False)
        radio.setDataRate(RF24_250KBPS)
        radio.setChannel(0x4c)

        radio.openReadingPipe(1, GATEWAY_ADDRESS)
        radio.startListening()
        return radio


    def run(self):
        self._windows_without_messages = 0
        self._window_end = time.time()

        while True:
            self._start_window()
            self._run_window()
            self._end_window()


    def _start_window(self):
        self._window_end += 60
        self._window_time = time.strftime('%H:%M', time.localtime(self._window_end))

        del self._temp_samples[:]
        del self._humidity_samples[:]


    def _run_window(self):
        while time.time() < self._window_end:
            if self._radio.available():
                self._handle_message(self._radio.read(3))
                self._blink_led()

            if sys.stdout.isatty():
                self._update_output()

            time.sleep(0.5)


    def _handle_message(self, msg):
        msg_id = msg[0]
        temp = msg[1]
        humidity = msg[2]

        if temp > 127:
            temp -= 256

        if msg_id == self._last_message_id:
            return

        self._last_message_id = msg_id
        self._temp_samples.append(temp)
        self._humidity_samples.append(humidity)


    def _end_window(self):
        self._update_output()
        sys.stdout.write('\n')
        sys.stdout.flush()

        if not self._temp_samples:
            self._windows_without_messages += 1

            if self._windows_without_messages >= RESTART_MINUTES:
                sys.exit('Exiting... (and hopefully being restarted by systemd)')

            return

        self._windows_without_messages = 0

        try:
            self._cloudwatch.put_metric_data(
                Namespace='Sauna',
                MetricData=[
                    {
                        'MetricName': 'Samples',
                        'Value': len(self._temp_samples)
                    },
                    {
                        'MetricName': 'Temperature',
                        'Value': mean(self._temp_samples)
                    },
                    {
                        'MetricName': 'Humidity',
                        'Value': mean(self._humidity_samples)
                    },
                ])
        except botocore.exceptions.ClientError as e:
            sys.stderr.write('Exception reporting metrics:\n{}\n'.format(e))


    def _update_output(self):
        if self._temp_samples:
            sys.stdout.write(' {} {:3d}C {:3d}% {:3d}#\r'.format(self._window_time,
                                                                 int(mean(self._temp_samples)),
                                                                 int(mean(self._humidity_samples)),
                                                                 len(self._temp_samples)))
        else:
            sys.stdout.write(' {} ---C ---% ---#\r'.format(self._window_time))


    def _blink_led(self):
        if self._can_blink_led:
            self._set_led(0)
            time.sleep(0.2)
            self._set_led(1)


    def _set_led(self, value):
        try:
            with open('/sys/class/leds/ACT/brightness', 'wt') as f:
                f.write(f'{value:d}')

            return True

        except IOError as e:
            sys.stdout.write(f'\nError setting LED to {value}: {e}\n')
            return False


if __name__ == '__main__':
    gw = SaunaGateway()

    gw._radio.printDetails()
    print('Sauna gateway started')
    print()
    sys.stdout.flush()

    gw.run()
