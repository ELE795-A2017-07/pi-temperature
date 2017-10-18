#! /usr/bin/env python3

import RPi.GPIO as GPIO
import time

mypin = 8

#See image at the bottom of https://www.raspberrypi.org/documentation/usage/gpio/
#for pin numbering
GPIO.setmode(GPIO.BOARD)

GPIO.setup(mypin, GPIO.OUT)

#Reset pulse
t0 = time.time()
GPIO.output(mypin, GPIO.LOW)
time.sleep(0.0005) #must keep low at least 480us
t0delta = time.time() - t0

#Read presence pulse
t1 = time.time()
GPIO.setup(mypin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
sleeptime = max(0, 0.00006 - (time.time() - t1))
time.sleep(sleeptime) #Must wait at least 60us
t1delta = time.time() - t1

starttime = time.time()
delta = None
endtime = time.time() + 0.000250 #Bus is held low for 60us to 240us
b_responded = False
while (not b_responded and time.time() < endtime):
	if (not GPIO.input(mypin)):
		b_responded = True
	if (delta is None):
		delta = time.time() - starttime

if (b_responded):
	print('Sensor responded')
else:
	print('Sensor didn\'t respond')

GPIO.cleanup()

print('delta = {:f}'.format(delta))
print('t0delta = {:f}'.format(t0delta))
print('t1delta = {:f}'.format(t1delta))
