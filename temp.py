#! /usr/bin/env python3

import RPi.GPIO as GPIO
import time

mypin = 8

#See image at the bottom of https://www.raspberrypi.org/documentation/usage/gpio/
#for pin numbering
GPIO.setmode(GPIO.BOARD)
GPIO.setup(mypin, GPIO.OUT)

while (True):
	GPIO.output(mypin, GPIO.HIGH)
	time.sleep(0.5)
	GPIO.output(mypin, GPIO.LOW)
	time.sleep(0.5)
