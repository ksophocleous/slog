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

#include "slog/slog.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ctime>
#include <iostream>
#include <iomanip>

using namespace slog;

std::string slog::getversion()		{ return strobj() << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH; }
uint32_t slog::getmajorversion()	{ return VERSION_MAJOR; }
uint32_t slog::getminorversion()	{ return VERSION_MINOR; }
uint32_t slog::getpatchversion()	{ return VERSION_PATCH; }

class static_init
{
	public:
		static_init(std::function<void()> init_func)
		{
			init_func();
		}
};

std::map<std::string, logconfig::print_func> logconfig::print_functions;

static static_init loginit([]()
{
	logconfig::print_functions.clear();
	logconfig::print_functions["default_console"] = logconfig::default_console_print;
});

//static
bool logconfig::timestamps = true;
bool logconfig::print_logtype = true;
bool logconfig::usecolor = true;
bool logconfig::print_priority = false;

#ifdef _WIN32
	static uint32_t ConvertToOSSpecificValue(consolecolor col)
	{
		switch (col)
		{
			case consolecolor::White: return 0x0f;
			case consolecolor::Gray: return 0x07;
			case consolecolor::Red: return 0x0c;
			case consolecolor::Green: return 0x0a;
			case consolecolor::Blue: return 0x09;
			case consolecolor::Yellow: return 0x0e;
			case consolecolor::Magenta: return 0x0d;
			case consolecolor::Cyan: return 0x0b;
		}

		return 0x0f;
	}
#endif

static const char* XTermColorSequence(consolecolor col)
{
	switch (col)
	{
		case consolecolor::White: return "\x1B[37;1m";
		case consolecolor::Gray: return "\x1B[37m";
		case consolecolor::Red: return "\x1B[31;1m";
		case consolecolor::Green: return "\x1B[32;1m";
		case consolecolor::Blue: return "\x1B[34;1m";
		case consolecolor::Yellow: return "\x1B[33;1m";
		case consolecolor::Magenta: return "\x1B[35;1m";
		case consolecolor::Cyan: return "\x1B[36;1m";
	}

	return "\x1B[0m";
}

//static
void logconfig::get_current_date_time(tm& timedate)
{
	time_t timeval;

	std::time(&timeval);

#ifdef _WIN32
	localtime_s(&timedate, &timeval);
#else
	localtime_r(&timeval, &timedate);
#endif
}

//static 
void logconfig::parse(int argc, char* argv[])
{
	static const std::string loglevelstr = "--log=";
	static const uint32_t loglevelstrsize = loglevelstr.length();

	for (int i = 0; i < argc; i++)
	{
		std::string param = argv[i];

		if (param.substr(0, loglevelstrsize).compare(loglevelstr) != 0)
			continue;
		
		std::string value(param.substr(loglevelstrsize));

		if (value.length() == 0)
			continue;
				
		const bool bEnable = (value[0] != '-');

		if (value[0] == '+' || value[0] == '-')
			value = value.substr(1);

		if (value.compare("info") == 0)
			info::type.enabled = bEnable;
		else if (value.compare("warn") == 0)
			warn::type.enabled = bEnable;
		else if (value.compare("error") == 0)
			error::type.enabled = bEnable;
		else if (value.compare("verbose") == 0)
			verbose::type.enabled = bEnable;
		else if (value.compare("debug") == 0)
			debug::type.enabled = bEnable;
		else if (value.compare("timestamps") == 0 || value.compare("timestamp") == 0)
			logconfig::timestamps = bEnable;
		else if (value.compare("color") == 0 || value.compare("colors") == 0)
			logconfig::usecolor = bEnable;
		else if (value.compare("labels") == 0 || value.compare("label") == 0)
			logconfig::print_logtype = bEnable;
	}
}

//static
std::string logconfig::formatmsg(const logtype& ltype, const std::string& msg)
{
	std::ostringstream ss;

	if (logconfig::timestamps)
	{
		tm tmstr;
		get_current_date_time(tmstr);

		std::ostringstream timestamp;
		timestamp << std::setw(4) << std::setfill('0') << (1900+tmstr.tm_year) << "-";
		timestamp << std::setw(2) << std::setfill('0') << (tmstr.tm_mon+1) << "-"; 
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_mday << " ";
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_hour << ":";
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_min << ":";
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_sec;

		ss << "[" << timestamp.str() << "] - ";
	}

	if (logconfig::print_logtype)
		ss << "[" << ltype << "] - ";
	
	ss << msg;

	return ss.str();
}

#pragma warning(disable:4996)

//static 
void logconfig::set_default_console_print(print_func defaultPrintFunc)
{
	logconfig::print_functions["default_console"] = (defaultPrintFunc) ? defaultPrintFunc : logconfig::default_console_print;
}

void logconfig::default_console_print(const logtype& type, const std::string& line)
{
	std::ostream& out = type.usestderr ? std::cerr : std::cout;

	if (logconfig::usecolor == false)
	{
		out << line << std::endl;
		return;
	}

	// figuring out the terminal is not easy and is very error prone - keep it simple for now
	// for example you can launch a %comspec% terminal from a mintty terminal and the %comspec% will have the environment variable TERM set to xterm
	// but of course %comspec% does not support ANSI color codes
	// I don't know of an easy way to identify that the console is %comspec% or not
	const std::string terminal = getenv("TERM");
	bool bShouldUseXTermColors = (terminal.compare("xterm") == 0);

#ifdef _WIN32
	// even if have the env variable set, if we are launched from visual studio it is possible that video studio was launched from an xterm compatible
	// console and has the environment variable still set, but the console wont recognize xterm ansi colors
	if (IsDebuggerPresent())
		bShouldUseXTermColors = false;
#endif

	if (bShouldUseXTermColors)
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

// template instantiation
template class logobj<logtype_info>;
template class logobj<logtype_warn>;
template class logobj<logtype_error>;
template class logobj<logtype_verbose>;
template class logobj<logtype_debug>;
template class logobj<logtype_success>;
