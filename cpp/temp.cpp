#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "GPIOClass.h"

using namespace std;

int main (void) {
	string inputstate;
	GPIOClass* mygpio = new GPIOClass("8"); //create new GPIO object to be attached to GPIO4

	mygpio->export_gpio(); //export GPIO
	cout << " GPIO pin(s) exported" << endl;

	gpio17->setdir_gpio("in"); //GPIO set to input
	cout << " GPIO pin direction(s) set" << endl;

	while (1) {
		usleep(500000); // wait for 0.5 seconds
		gpio17->getval_gpio(inputstate); //read state of GPIO17 input pin
		cout << "Current input pin state is " << inputstate <<endl;
		if (inputstate == "0") { // if input pin is at state "0" i.e. button pressed
			cout << "input pin state is "Pressed ".n Will check input pin state again in 20ms "<<endl;
				usleep(20000);
					cout << "Checking again ....." << endl;
					gpio17->getval_gpio(inputstate); // checking again to ensure that state "0" is due to button press and not noise
			if (inputstate == "0") {
				cout << "input pin state is definitely "Pressed". Turning LED ON" <<endl;
				gpio4->setval_gpio("1"); // turn LED ON

				cout << " Waiting until pin is unpressed....." << endl;
				while (inputstate == "0") {
					gpio17->getval_gpio(inputstate);
				};
				cout << "pin is unpressed" << endl;

			} else {
				cout << "input pin state is definitely "UnPressed". That was just noise." <<endl;
			}
		}
		gpio4->setval_gpio("0");

	}
	cout << "Exiting....." << endl;
	return 0;
}
