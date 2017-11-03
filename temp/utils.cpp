#include "utils.h"

chrono::time_point<chrono::system_clock> get_time_point(int us_delay) {
	return chrono::system_clock::now() + chrono::microseconds(us_delay);
}

void busywait_until(chrono::time_point<chrono::system_clock> end_tp) {
	while (chrono::system_clock::now() < end_tp) {
		//Busywait, do nothing
	}
}

void busywait(int us_delay) {
	busywait_until(get_time_point(us_delay));
}
