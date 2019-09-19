/*
* Copyright (c) 2019 lf <software@lfcode.ca>
* SPDX-License-Identifier: MIT
*/
// this is probably sinful and bad, but I can't figure out dynamically linking it
#pragma comment(lib, "dxva2.lib")

#include "system_includes.h"

template<typename T>
T clamp(T min, T max, T val) {
	return val < min ? min : val > max ? max : val;
}

struct ParseResult {
	int32_t brightnessChange;
	bool relativeMode;
};

auto parseArgs(int argc, char** argv) {
	using res = std::optional<ParseResult>;

	bool relativeMode;
	try {
		cxxopts::Options optparser{ argv[0], "Change display brightness, by default in increments" };
		optparser.positional_help("brightness").show_positional_help();

		int32_t change = 0;
		optparser.add_options()
			("a,absolute", "Set brightness to a given value")
			("brightness", "Amount to set brightness", cxxopts::value<std::int32_t>(change))
			("h,help", "Print help");

		optparser.parse_positional({ "brightness" });

		auto args = optparser.parse(argc, argv);
		if (args.count("help") || args.count("brightness") != 1) {
			std::cerr << optparser.help();
			return res{};
		}

		relativeMode = !(args.count("absolute"));
		auto brightnessChange = args["brightness"].as<std::int32_t>();
		return res{ParseResult{ brightnessChange, relativeMode }};
	}
	catch (cxxopts::OptionException exc) {
		std::cerr << exc.what() << "\n";
		return res{};
	}
}

int main(int argc, char **argv)
{
	auto parsedOpt = parseArgs(argc, argv);
	if (!parsedOpt.has_value()) {
		exit(1);
	}
	auto parsed = parsedOpt.value();

	std::cout << "Relative mode: " << parsed.relativeMode << "\n";
	std::cout << "Brightness change: " << parsed.brightnessChange << "\n";

	EnumDisplayMonitors(
		NULL, NULL,
		[](HMONITOR mon, auto, auto, auto cmdlineArgsRef) -> _BOOL {
			auto cmdlineArgs = *reinterpret_cast<ParseResult *>(cmdlineArgsRef);

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
					DWORD min, curr, max, requested;
					if (!GetMonitorBrightness(pm.hPhysicalMonitor, &min, &curr, &max)) {
						std::cerr << "Ignoring monitor: cannot get brightness: " << GetLastError() << "\n";
						return;
					}
					if (cmdlineArgs.relativeMode) {
						requested = curr + cmdlineArgs.brightnessChange;
					}
					else {
						requested = cmdlineArgs.brightnessChange;
					}
					DWORD newVal = clamp(min, max, requested);
					std::wcerr << L"Changing monitor " << pm.szPhysicalMonitorDescription << L" from " << curr << L" to " << newVal << L"\n";
					SetMonitorBrightness(pm.hPhysicalMonitor, newVal);
				}
			);
			DestroyPhysicalMonitors(numMonitors, monitors.data());
			return TRUE;
		},
		(reinterpret_cast<LPARAM> (&parsed))
	);
}
