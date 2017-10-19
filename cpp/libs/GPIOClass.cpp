#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "GPIOClass.h"

using namespace std;

GPIOClass::GPIOClass() {
	GPIOClass::construct("8"); //GPIO8 is default
}

GPIOClass::GPIOClass(string gnum) {
	GPIOClass::construct(gnum);
}

void GPIOClass::construct(string gnum) {
	this->gpionum = gnum; //Instantiate GPIOClass object for GPIO pin number "gnum"
}

int GPIOClass::export_gpio() {
	string export_str = "/sys/class/gpio/export";
	ofstream exportgpio(export_str.c_str()); // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
	if (exportgpio < 0) {
		cout << " OPERATION FAILED: Unable to export GPIO"<< this->gpionum <<" ."<< endl;
		return -1;
	}

	exportgpio << this->gpionum ; //write GPIO number to export
	exportgpio.close(); //close export file
	return 0;
}

int GPIOClass::unexport_gpio() {
	string unexport_str = "/sys/class/gpio/unexport";
	ofstream unexportgpio(unexport_str.c_str()); //Open unexport file
	if (unexportgpio < 0) {
		cout << " OPERATION FAILED: Unable to unexport GPIO"<< this->gpionum <<" ."<< endl;
		return -1;
	}

	unexportgpio << this->gpionum ; //write GPIO number to unexport
	unexportgpio.close(); //close unexport file
	return 0;
}

int GPIOClass::setdir_gpio(string dir) {
	if (!this->dir_s.is_open()) {
		string setdir_str ="/sys/class/gpio/gpio" + this->gpionum + "/direction";
		this->dir_s.open(setdir_str.c_str()); // open direction file for gpio

		if (!this->dir_s) {
			cout << " OPERATION FAILED: Unable to set direction of GPIO"<< this->gpionum <<" ."<< endl;
			return -1;
		}
	}

	this->dir_s << dir; //write direction to direction file
	return 0;
}

int GPIOClass::setval_gpio(string val) {
	if (!this->data_s.is_open()) {
		string setval_str = "/sys/class/gpio/gpio" + this->gpionum + "/value";
		this->data_s.open(setval_str.c_str()); // open value file for gpio
		if (!this->data_s) {
			cout << " OPERATION FAILED: Unable to set the value of GPIO"<< this->gpionum <<" ."<< endl;
			return -1;
		}
	}

	this->data_s << val << flush; //write value to value file
	return 0;
}

int GPIOClass::getval_gpio(string& val) {
	if (!this->data_s.is_open()) {
		string getval_str = "/sys/class/gpio/gpio" + this->gpionum + "/value";
		this->data_s.open(getval_str.c_str()); // open value file for gpio
		if (!this->data_s) {
			cout << " OPERATION FAILED: Unable to get value of GPIO"<< this->gpionum <<" ."<< endl;
			return -1;
		}
	}

	this->data_s >> val ; //read gpio value

	if (val != "0") {
		val = "1";
	} else {
		val = "0";
	}

	return 0;
}

string GPIOClass::get_gpionum() {
	return this->gpionum;
}
