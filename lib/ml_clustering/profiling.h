#pragma once

#include <iostream>
#include <chrono>

struct Timer {


	std::string name;
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point endTime;


	Timer(std::string name)
	: name(name) {

		startTime = std::chrono::high_resolution_clock::now();

	}

	~Timer() {
		endTime = std::chrono::high_resolution_clock::now();
		auto timeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

		std::cout << "-------------------- " << name << " --------------------" << std::endl;
		std::cout << "	Duration: " << timeDelta << " milliseconds\n" << std::endl;

	}

};