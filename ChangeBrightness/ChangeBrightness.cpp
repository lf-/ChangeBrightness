/*
* Copyright (c) 2019 lf <software@lfcode.ca>
* SPDX-License-Identifier: MIT
*/
// this is probably sinful and bad, but I can't figure out dynamically linking it
#pragma comment(lib, "dxva2.lib")

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <execution>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>


template<typename T>
T clamp(T min, T max, T val) {
	return val < min ? min : val > max ? max : val;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " brightness_change\n";
		return 1;
	}
	std::istringstream iss { argv[1] };

	int change;
	iss >> change;
	if (iss.fail()) {
		std::cerr << "Failed to convert input" << argv[1] << " to integer\n";
		return 1;
	}

	EnumDisplayMonitors(
		NULL, NULL,
		[](HMONITOR mon, auto _, auto _2, auto theChange) -> _BOOL {
			DWORD numMonitors;
			if (!GetNumberOfPhysicalMonitorsFromHMONITOR(mon, &numMonitors)) {
				std::cerr << "Ignoring monitor: cannot get number of physical monitors: " << GetLastError() << "\n";
				return TRUE;
			}
			std::vector<PHYSICAL_MONITOR> monitors;
			monitors.resize(numMonitors);
			if (!GetPhysicalMonitorsFromHMONITOR(mon, numMonitors, monitors.data())) {
				std::cerr << "Ignoring monitor: cannot get physical monitors: " << GetLastError() << "\n";
				return TRUE;
			}
			std::for_each(std::execution::par_unseq, monitors.begin(), monitors.end(), [=](auto&& pm) {
					DWORD min, curr, max;
					if (!GetMonitorBrightness(pm.hPhysicalMonitor, &min, &curr, &max)) {
						std::cerr << "Ignoring monitor: cannot get brightness: " << GetLastError() << "\n";
						return;
					}
					DWORD newVal = clamp(min, max, curr + theChange);
					std::wcerr << L"Changing monitor " << pm.szPhysicalMonitorDescription << L" from " << curr << L" to " << newVal << L"\n";
					SetMonitorBrightness(pm.hPhysicalMonitor, newVal);
				}
			);
			DestroyPhysicalMonitors(numMonitors, monitors.data());
			return TRUE;
		},
		change
	);
}
