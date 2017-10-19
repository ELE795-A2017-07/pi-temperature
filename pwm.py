#! /usr/bin/env python3

import RPi.GPIO as GPIO
import time

mypin = 8

#See image at the bottom of https://www.raspberrypi.org/documentation/usage/gpio/
#for pin numbering
GPIO.setmode(GPIO.BOARD)

GPIO.setup(mypin, GPIO.OUT)

p = GPIO.PWM(mypin, 10000000)
p.start(50)

while (True):
	pass
	#GPIO.output(mypin, GPIO.LOW)
	#GPIO.output(mypin, GPIO.HIGH)

p.stop()
GPIO.cleanup()
