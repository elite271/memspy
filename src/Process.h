#pragma once

#include "Platform.h"
#include <vector>
#include <string>


class Process
{
public:
	Process();
	~Process();

	std::string currentProcessName;
	DWORD processID = 0;
	uintptr_t moduleAddress;

	// TODO: make getters to make this more secure

private:

};

