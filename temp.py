#! /usr/bin/env python3

import RPi.GPIO as GPIO
import time

TEMP_SENSOR_PIN = 8

def busywait(endtime):
	while (time.time() < endtime):
		pass

def init():
	#See image at the bottom of https://www.raspberrypi.org/documentation/usage/gpio/
	#for pin numbering
	GPIO.setmode(GPIO.BOARD)

	GPIO.setup(TEMP_SENSOR_PIN, GPIO.OUT)

def oscope_trigger():
	"""This sends a recognizable pattern for the oscilloscope to trigger on"""
	GPIO.output(TEMP_SENSOR_PIN, GPIO.LOW)
	GPIO.output(TEMP_SENSOR_PIN, GPIO.HIGH)
	GPIO.output(TEMP_SENSOR_PIN, GPIO.LOW)
	GPIO.output(TEMP_SENSOR_PIN, GPIO.HIGH)
	GPIO.output(TEMP_SENSOR_PIN, GPIO.LOW)
	GPIO.output(TEMP_SENSOR_PIN, GPIO.HIGH)

def reset_pulse():
	#Reset pulse
	t1 = time.time() + 0.0005 #must keep low at least 480us
	GPIO.output(TEMP_SENSOR_PIN, GPIO.LOW)
	busywait(t1)

def read_presence():
	#Read presence pulse
	# Sensor waits 15us to 60us then pulls low for 60us to 240us
	endtime = time.time() + 0.00006 + 0.000250
	GPIO.setup(TEMP_SENSOR_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

	b_responded = False
	while (not b_responded and time.time() < endtime):
		if (not GPIO.input(TEMP_SENSOR_PIN)):
			b_responded = True
	return b_responded

def cleanup():
	GPIO.cleanup()

init()
oscope_trigger()
reset_pulse()

b_responded = read_presence()
if (b_responded):
	print('Sensor responded')
else:
	print('Sensor didn\'t respond')

cleanup()
