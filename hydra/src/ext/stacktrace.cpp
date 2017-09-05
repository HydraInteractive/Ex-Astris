// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Functions for getting the amount of RAM that Hydra is using.
 *
 * License: Mozilla Public License Version 2.0 (https://www.mozilla.org/en-US/MPL/2.0/ OR See accompanying file LICENSE)
 * Authors:
 *  - Dan Printzell
 */
#include <hydra/ext/stacktrace.hpp>

#include <cstdint>
#include <sstream>
#include <cstring>

#ifdef __linux__
#include <cxxabi.h>
extern "C" {
#include <execinfo.h>
}

std::string Hydra::Ext::getStackTrace() {
	const uint32_t maxFrames = 63;
	std::stringstream ss;
  ss << "stack trace:\n";

	// storage array for stack trace address data
	void* addrlist[maxFrames+1];

	// retrieve current stack addresses
	uint32_t addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

	if (addrlen == 0)
		return ss.str();

	// create readable strings to each frame.
	char** symbollist = backtrace_symbols(addrlist, addrlen);

	// print the stack trace.
	for (uint32_t i = 4; i < addrlen; i++) {
		char* symbol = symbollist[i];

		char *openP, *plus;

		int status;
		size_t size = 0x1000;
		char output[0x1000];
		char* ret;

		openP = strstr(symbol, "(");
		if (!openP)
			goto notFound;

		plus = strstr(openP, "+");
		if (plus == openP + 1 || !plus)
			goto notFound;
		*plus = '\0';

		ret = abi::__cxa_demangle(openP + 1, output, &size, &status);
		*plus = '+';
		if (status)
			goto notFound;

		*openP = '\0';
		ss << "\t" << symbol << '(' << ret << '+' << (plus + 1) << "\n";
		*openP = '('; // -V519
		continue;

	notFound:
		ss << "\t" << symbol << "\n";
	}

	free(symbollist);
	return ss.str();
}

#else
// #warning "Stacktrace missing!"
std::string Hydra::Ext::getStackTrace() { return ""; }
#endif
