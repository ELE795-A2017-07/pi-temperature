#! /usr/bin/env python3

import RPi.GPIO as GPIO
import time

mypin = 8

#See image at the bottom of https://www.raspberrypi.org/documentation/usage/gpio/
#for pin numbering
GPIO.setmode(GPIO.BOARD)
GPIO.setup(mypin, GPIO.OUT)

GPIO.setup(10, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
in10 = GPIO.input(10)
in12 = GPIO.input(12)
print('Pin 10 was {:d} (should be LOW), pin 12 was {:d} (should be HIGH)'.format(in10, in12))

while (True):
	GPIO.output(mypin, GPIO.HIGH)
	time.sleep(0.5)
	GPIO.output(mypin, GPIO.LOW)
	time.sleep(0.5)
