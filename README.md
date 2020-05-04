
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

Clone https://github.com/nRF24/RF24 on a Raspi and build it:

    ./configure --driver=SPIDEV
    make
    sudo make install

Create a virtualenv for the Gateway:

    virtualenv -p python3.7 ~/sauna-gateway

Install the RF24 python module in it from the cloned project above:

    cd RF24/pyRF24
    ~/sauna-gateway/bin/python setup.py install

Run the code with the virtualenv python:

    ~/sauna-gateway/bin/python sauna-gateway.py


# Electronics

## nRF24L01 connection

I've used this colour-coding of cables to hook up the nRF24L01 radios:

| Colour | Function | Arduino Micro | Raspberry Pi 
|--------|--------- |---------------|----------------
| Red    | 3.3V     |               | 
| Black  | Ground   |               |
| Orange | CSN      | D12           | GPIO 8  / CEO
| Brown  | CE       | D11           | GPIO 22 / GP3
| Green  | MOSI     |               |
| Yellow | SCLK     |               |
| Purple | INT      | Not used      |
| Blue   | MISO     |               |


## Sensor

Arduino Micro with a DHT22 sensor.

| Pin | Function
|-----|--------------
| D2  | LCD RS
| D3  | LCD E
| D4  | LCD D4
| D5  | LCD D5
| D6  | LCD D6
| D7  | LCD D7
| D9  | DHT22 data
| D11 | RF24 CE
| D12 | RF24 CSN
| D13 | Status led
