INSTALL:

clone wiringPi into this directory:
$ git clone git://git.drogon.net/wiringPi wiringPi

compile it:
$ (cd wiringPi && ./build)

Afterwards, build the project and use it:
$ make temp && sudo ./temp

REFERENCES:
 * wiringPi pin numbering: https://projects.drogon.net/raspberry-pi/wiringpi/pins/
 * Raspberry Pi physical pin numbering: https://www.raspberrypi.org/documentation/usage/gpio/
