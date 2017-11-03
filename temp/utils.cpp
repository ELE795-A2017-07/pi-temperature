#include "utils.h"

std::chrono::time_point<std::chrono::system_clock> get_time_point(int us_delay) {
	return std::chrono::system_clock::now() + std::chrono::microseconds(us_delay);
}

void busywait_until(std::chrono::time_point<std::chrono::system_clock> end_tp) {
	while (std::chrono::system_clock::now() < end_tp) {
		//Busywait, do nothing
	}
}

void busywait(int us_delay) {
	busywait_until(get_time_point(us_delay));
}
