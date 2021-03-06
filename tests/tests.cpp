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

//#define SLOG_DISABLE 1
//#define SLOG_DISABLE_INFO 1

#include <slog/slog.h>
#include <slog/slog_logdevice_file.h>
#include <slog/slog_logdevice_console.h>
#include <slog/slog_logdevice_custom_function.h>

#ifdef _MSC_VER
#define unlink _unlink
#else
#include <unistd.h>
#endif

#include <cerrno>
#include <cstdlib>

void compare_file_contents(const char* filename, std::string contents, std::string errorstring)
{
	std::ifstream file(filename);
	if (file.bad())
		throw std::runtime_error(strobj() << "compare_file_contents :: file '" << filename << "'");

	std::string value;
	file >> value;

	if (value.compare(contents) != 0)
		throw std::runtime_error(strobj() << errorstring);
}

/// --- tests

void emptylog(int argc, char* argv[])
{
	const char logfilename[] = "emptylog.test.log";
	if (unlink(logfilename) != 0 && (errno != ENOENT))
		throw std::runtime_error(strobj() << "emptylog :: failed to delete file '" << logfilename << "'");

	{
		slog::logdevice_file logfile(logfilename, true);
		slog::info::type.enabled = false;
		slog::info() << "test";
		slog::info::type.enabled = true;
	}

	compare_file_contents(logfilename, "", "type.enabled = false; failed not to write to log file");
}

void simple_log_line(int argc, char* argv[])
{
	const char thisline[] = "this simple line";

	slog::logconfig curconfig;
	curconfig.timestamps = false;
	curconfig.print_logtype = false;

	slog::logdevice_custom_function customlog("console", 
		[&thisline](const slog::logtype& type, const std::string& line)
		{
			if (line.compare(thisline) != 0)
				throw std::runtime_error(strobj() << "simple_log_line :: failed compose a simple log line with all prefixes removed");
		});

	slog::info() << thisline;
}

void default_verbose_debug_off(int argc, char* argv[])
{
	slog::logdevice_custom_function customlog("console",
		[](const slog::logtype& type, const std::string& line)
		{
			if (line.length() > 0)
				throw std::runtime_error(strobj() << "default_verbose_debug_off :: default state of verbose and debug is not disabled");
		});

	slog::debug() << "testing";
	slog::verbose() << "testing again";
}

void empty_lines_should_print(int argc, char* argv[])
{
	slog::logdevice_custom_function customlog("console",
		[](const slog::logtype& type, const std::string& line)
		{
			if (line.length() == 0)
				throw std::runtime_error(strobj() << "empty_lines_should_print :: empty info line printed nothing (must print prefixes since they are enabled)");
		});

	slog::info();
}

// -------------------------------------------------------------------------------------

#define TIMES 20000

#include <iostream>

void bench(int argc, char* argv[])
{
	std::ostringstream ss;
	
	slog::logconfig curconfig;
	curconfig.timestamps = curconfig.print_logtype = curconfig.usecolor = false;

	for (int i = 1; i < argc; i++)
	{
		ss << argv[i] << " ";
	}

	if (ss.str().find("-t1") != std::string::npos)
	{
		for (uint32_t i = 0; i < TIMES; i++)
			printf("complex %s %d %f\n", "string", 10, 30.f);

		exit(0);
	}
	else if (ss.str().find("-t2") != std::string::npos)
	{
		for (uint32_t i = 0; i < TIMES; i++)
			slog::info() << "complex " << "string" << " " << 10 << " " << 30.001f;

		exit(1);
	}
	else if (ss.str().find("-t3") != std::string::npos)
	{
		for (uint32_t i = 0; i < TIMES; i++)
			std::cout << "complex " << "string" << " " << 10 << " " << 30.001f << std::endl;

		exit(1);
	}
}

int main(int argc, char* argv[])
{
	bench(argc, argv);

	try
	{
		emptylog(argc, argv);
		simple_log_line(argc, argv);
		default_verbose_debug_off(argc, argv);
		empty_lines_should_print(argc, argv);

		slog::logconfig benchconfig(argc, argv);
		slog::verbose::type.enabled = true;
		slog::debug::type.enabled = true;

		{
			slog::logdevice_file keep_this_instance_around("logoutput.log", true);

			slog::info();
			slog::info() << "---------------- NEW RUN ----------------";
			slog::info() << "slog version " << slog::getmajorversion() << "." << slog::getminorversion() << "." << slog::getpatchversion();

			slog::error() << "major error: failed to say hello to world";
			slog::warn() << "warning: this should be yellow";
			slog::info() << "fyi: life is full of meaningless little lines";
			slog::verbose() << "verbosity: also known as 'it goes to eleven'";
			slog::debug() << "debugging: for when verbose just isn't enough";
			slog::success() << "success: you only see this when you get lucky";
		}

		slog::debug() << "major debug reporting for duty... this log wont make in the log file";

		slog::success() << "all tests passed";

		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "Test failed: " << e.what() << std::endl;
		return -1;
	}
}
