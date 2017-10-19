#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

using namespace std;

int main (void) {
	const int TEMP_SENSOR_PIN = 15;

	wiringPiSetup();
	pinMode(TEMP_SENSOR_PIN, OUTPUT);

	while (1) {
		digitalWrite(TEMP_SENSOR_PIN, HIGH);
		digitalWrite(TEMP_SENSOR_PIN, LOW);
	}
	cout << "Exiting....." << endl;
	return 0;
}
