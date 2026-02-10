#pragma once

#include <vector>
#include "Process.h"

class ProcessList
{
public:
	ProcessList();
	~ProcessList();

	bool init();

	bool Refresh();

	void Clear();

	Process at(size_t i);

	size_t size();

private:

	bool scanForProcesses();

private:
	std::vector<Process> processes;
};


