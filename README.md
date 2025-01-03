
# internet-of-saunas

This is a small sensor for a sauna (or anything else that needs remote
temperature monitoring) that measures temperature and humidity and
sends out the readings over an nRF24L01 radio chip.

A Raspberry Pi with a corresponding chip runs a gateway that receives
the radio messages and logs them in AWS CloudWatch as custom metrics.

The CloudWatch metrics can be used to plot graphs for dashboards (and
exporting them as PNG images with a small AWS lambda function) or send
alarms when the sauna has reached the desired temperature.


# Gateway setup

Enable SPI in the Interfaces section of `raspi-config`, if not already done:

    sudo raspi-config

Install required Raspbian packages:

    apt-get install libboost-python-dev

Clone https://github.com/nRF24/RF24 on a RaspberryPi and build it:

    ./configure --driver=SPIDEV
    make
    sudo make install

Create a venv for the Gateway:

    python -m venv ~/sauna-gateway

Install the RF24 python module in it from the cloned project above:

    cd RF24/pyRF24
    ~/sauna-gateway/bin/python setup.py install

Install the AWS Python SDK:

    ~/sauna-gateway/bin/pip install boto3

Run the code with the virtualenv python:

    ~/sauna-gateway/bin/python sauna-gateway.py

# Electronics

## nRF24L01 connection

Pinout on the particular chip + antenna component I've got.  Seen from
component side with the antenna facing up, i.e. this is the pinout for
the connector on the circuit board:

| VCC | CSN | MOSI | INT  |
| GND | CE  | SCLK | MISO |


I've used this colour-coding of cables to hook up the nRF24L01 radios:

| Colour | Function
|--------|---------
| Red    | 3.3V
| Black  | Ground
| Orange | CSN
| Brown  | CE
| Green  | MOSI
| Yellow | SCLK
| Purple | INT
| Blue   | MISO


## Sensor

Arduino Uno:

| Pin | Function
|-----|--------------
| D2  | LCD RS
| D3  | LCD E
| D4  | LCD D4
| D5  | LCD D5
| D6  | LCD D6
| D7  | LCD D7
| D8  | DHT22 data
| D9  | RF24 CE
| D10 | RF24 CSN
| D11 | RF24 MOSI
| D12 | RF24 MISO
| D13 | RF24 SCLK


LCD display pinout:

| Pin | Name | Connection
|-----|------|-----------
|   1 | VSS  | GND
|   2 | VDD  | 5V
|   3 | V0   | Middle pin of a potentiometer between 5V and GND
|   4 | RS   | D2
|   5 | RW   | GND
|   6 | E    | D3
|   7 | D0   | -
|   8 | D1   | -
|   9 | D2   | -
|  10 | D3   | -
|  11 | D4   | D4
|  12 | D5   | D5
|  13 | D6   | D6
|  14 | D7   | D7
|  15 | A/L+ | 5V via 220 ohm resistor
|  16 | K/L- | GND


## Gateway

Raspberry Pi model B+ (?).

Sensor is wired up to standard SPI pins, plus:

| CSN | GPIO 8 (CE0) |
| CE  | GPIO 25 (GP5) |
