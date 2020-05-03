#!/bin/env python3

import time
import sys
from RF24 import *

GATEWAY_ADDRESS = b'IoSGW'
CE_PIN = 22

def main():
    # Use /dev/spidev0.0
    radio = RF24(CE_PIN, 0)

    radio.begin()

    radio.payloadSize = 3
    radio.setAutoAck(False)
    radio.setDataRate(RF24_250KBPS)
    radio.setChannel(0x4c)

    radio.openReadingPipe(1, GATEWAY_ADDRESS)
    radio.startListening()

    radio.printDetails()

    while True:
        time.sleep(1)

        if radio.available():
            msg = radio.read(3)
            print('Received {} {} {}'.format(*msg))


if __name__ == '__main__':
    main()
