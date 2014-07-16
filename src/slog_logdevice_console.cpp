//================================================================================
//
//	The MIT License (MIT)
//
//	Copyright (c) 2014 Konstantinos Sofokleous
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//
//================================================================================

#include "slog/slog_logdevice_console.h"

#pragma warning(disable:4996)

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>

using namespace slog;

#ifdef _WIN32
static uint32_t ConvertToOSSpecificValue(consolecolor col)
{
	switch (col)
	{
		case consolecolor::white: return 0x0f;
		case consolecolor::gray: return 0x07;
		case consolecolor::red: return 0x0c;
		case consolecolor::green: return 0x0a;
		case consolecolor::blue: return 0x09;
		case consolecolor::yellow: return 0x0e;
		case consolecolor::magenta: return 0x0d;
		case consolecolor::cyan: return 0x0b;
	}

	return 0x0f;
}
#endif

static const char* XTermColorSequence(consolecolor col)
{
	switch (col)
	{
		case consolecolor::white: return "\x1B[37;1m";
		case consolecolor::gray: return "\x1B[37m";
		case consolecolor::red: return "\x1B[31;1m";
		case consolecolor::green: return "\x1B[32;1m";
		case consolecolor::blue: return "\x1B[34;1m";
		case consolecolor::yellow: return "\x1B[33;1m";
		case consolecolor::magenta: return "\x1B[35;1m";
		case consolecolor::cyan: return "\x1B[36;1m";
	}

	return "\x1B[0m";
}

logdevice_console::logdevice_console() : logdevice("console")
{
	// figuring out the terminal is not easy and is very error prone - keep it simple for now
	// for example you can launch a %comspec% terminal from a mintty terminal and the %comspec% will have the environment variable TERM set to xterm
	// but of course %comspec% does not support ANSI color codes
	// I don't know of an easy way to identify that the console is %comspec% or not
	const char* envvar = getenv("TERM");
	const std::string terminal = envvar ? envvar : "";
	_xterm_console = (terminal.compare("xterm") == 0);

#ifdef _WIN32
	// even if have the env variable set, if we are launched from visual studio it is possible that video studio was launched from an xterm compatible
	// console and has the environment variable still set, but the console wont recognize xterm ansi colors
	if (IsDebuggerPresent())
		_xterm_console = false;
#endif
}

void logdevice_console::writelogline(const logtype& type, const std::string& line)
{
	std::ostream& out = type.usestderr ? std::cerr : std::cout;

	if (logconfig::_cur_config->usecolor == false)
	{
		out << line << std::endl;
		return;
	}

	if (_xterm_console)
	{
		static const char* CC_REMOVE = "\x1B[0m";
		out << XTermColorSequence(type.color) << line << CC_REMOVE << std::endl;
	}
#ifdef _WIN32
	else
	{
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hstdout, &csbi);
		SetConsoleTextAttribute(hstdout, ConvertToOSSpecificValue(type.color));
		out << line << std::endl;
		SetConsoleTextAttribute(hstdout, csbi.wAttributes);
	}
#else
	else
	{
		out << line << std::endl;
	}
#endif
}
