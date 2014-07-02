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
#include "slog/slog_logdevice_console.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ctime>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>

using namespace slog;

namespace slog
{
	std::string getversion()	{ return strobj() << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH; }
	uint32_t getmajorversion()	{ return VERSION_MAJOR; }
	uint32_t getminorversion()	{ return VERSION_MINOR; }
	uint32_t getpatchversion()	{ return VERSION_PATCH; }
}

/////////////////////////////////////////////////////////////////////

const logconfig* logconfig::_cur_config = nullptr;

static void set_logconfig_defaults(logconfig& conf)
{
	conf.timestamps = true;
	conf.print_logtype = true;
	conf.usecolor = true;
	conf.print_priority = false;
}

logconfig::logconfig()
{
	set_logconfig_defaults(*this);
	_prev_config = _cur_config;
	_cur_config = this;
}

logconfig::logconfig(int argc, char* argv[])
{
	set_logconfig_defaults(*this);
	_prev_config = _cur_config;
	_cur_config = this;

	parse(argc, argv);
}

logconfig::~logconfig()
{
	_cur_config = _prev_config;
}

/////////////////////////////////////////////////////////////////////

std::map<std::string, logdevice*> logconfig::print_functions;

logdevice_console _default_console_logdevice;
logconfig _default_logconfig;

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
			timestamps = bEnable;
		else if (value.compare("color") == 0 || value.compare("colors") == 0)
			usecolor = bEnable;
		else if (value.compare("labels") == 0 || value.compare("label") == 0)
			print_logtype = bEnable;
	}
}

//static
std::string logconfig::formatmsg(const logtype& ltype, const std::string& msg)
{
	assert(_cur_config != nullptr);

	std::ostringstream ss;

	if (_cur_config->timestamps)
	{
		tm tmstr;
		time_t timeval;

		std::time(&timeval);

#ifdef _WIN32
		localtime_s(&tmstr, &timeval);
#else
		localtime_r(&timeval, &tmstr);
#endif

		std::ostringstream timestamp;
		timestamp << std::setw(4) << std::setfill('0') << (1900+tmstr.tm_year) << "-";
		timestamp << std::setw(2) << std::setfill('0') << (tmstr.tm_mon+1) << "-"; 
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_mday << " ";
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_hour << ":";
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_min << ":";
		timestamp << std::setw(2) << std::setfill('0') << tmstr.tm_sec;

		ss << "[" << timestamp.str() << "] - ";
	}

	if (_cur_config->print_logtype)
		ss << "[" << ltype << "] - ";
	
	ss << msg;

	return ss.str();
}

#pragma warning(disable:4996)

// template instantiation
template class logobj<logtype_info>;
template class logobj<logtype_warn>;
template class logobj<logtype_error>;
template class logobj<logtype_verbose>;
template class logobj<logtype_debug>;
template class logobj<logtype_success>;
