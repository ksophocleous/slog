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

#include "slog/slog_logdevice_file.h"

using namespace slog;

logdevice_file::logdevice_file(const std::string& filename, bool bAppend) : logdevice("logdevice_file")
{
	auto mode = (bAppend) ? (std::ios::out | std::ios::app) : std::ios::out;
	m_file.open(filename.c_str(), mode);
	if (m_file.good() == false)
		throw std::runtime_error(strobj() << "failed to open log file '" << filename << "' for write");
}

void logdevice_file::writelogline(const logtype& type, const std::string& line)
{
	m_file << line << std::endl;
}