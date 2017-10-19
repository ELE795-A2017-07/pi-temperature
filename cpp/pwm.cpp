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

	mygpio->setdir_gpio("in"); //GPIO set to input
	cout << " GPIO pin direction(s) set" << endl;

	while (1) {
		mygpio->setval_gpio("1");
		mygpio->setval_gpio("0");
	}
	cout << "Exiting....." << endl;
	return 0;
}
