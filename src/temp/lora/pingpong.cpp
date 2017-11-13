/*
 *  simple ping-pong test by requesting an ACK from the gateway
 *
 *  Copyright (C) 2016 Congduc Pham, University of Pau, France
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 * last update: Nov. 16th by C. Pham
 */
#include <iostream>

// edit this file to the right configuration parameters
#include "config.h"
#include "lora.h"

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE LORA MODE, NODE ADDRESS
#define LORA_MODE  1
#define node_addr 8
//////////////////////////////////////////////////////////////////

#define DEFAULT_DEST_ADDR 1


int main(void) {
	int e;

	LoRa::init(LORA_MODE, DEFAULT_CHANNEL, node_addr);
	LoRa::setup_exchange();
	while (true) {
		std::cout << "Sending Ping" << std::endl;
		e = LoRa::exchange(DEFAULT_DEST_ADDR);
		switch (e) {
		case 0:
			std::cout << "Pong received from gateway!" << std::endl;
			break;
		case 3:
			std::cout << "No Pong!" << std::endl;
			break;
		}
		delay(10000);
	}
	return 0;
}
