#! /usr/bin/env python3

import RPi.GPIO as GPIO
import time

def busywait(endtime):
	while (time.time() < endtime):
		pass

mypin = 8

#See image at the bottom of https://www.raspberrypi.org/documentation/usage/gpio/
#for pin numbering
GPIO.setmode(GPIO.BOARD)

GPIO.setup(mypin, GPIO.OUT)

GPIO.output(mypin, GPIO.LOW)
GPIO.output(mypin, GPIO.HIGH)
GPIO.output(mypin, GPIO.LOW)
GPIO.output(mypin, GPIO.HIGH)
GPIO.output(mypin, GPIO.LOW)
GPIO.output(mypin, GPIO.HIGH)

#Reset pulse
t1 = time.time() + 0.0005 #must keep low at least 480us
GPIO.output(mypin, GPIO.LOW)
busywait(t1)

#Read presence pulse
# Sensor waits 15us to 60us then pulls low for 60us to 240us
endtime = time.time() + 0.00006 + 0.000250
GPIO.setup(mypin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

b_responded = False
while (not b_responded and time.time() < endtime):
	if (not GPIO.input(mypin)):
		b_responded = True

if (b_responded):
	print('Sensor responded')
else:
	print('Sensor didn\'t respond')

GPIO.cleanup()
