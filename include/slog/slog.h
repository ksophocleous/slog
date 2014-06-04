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

#pragma once

#include <cstdint>
#include <sstream>
#include <functional>
#include <fstream>
#include <map>

#ifndef SLOG_NO_COPY
#define SLOG_NO_COPY 1
#endif

#ifndef SLOG_EXCEPTION_PRINT 
#define SLOG_EXCEPTION_PRINT 1
#endif

class strobj
{
	public:
		strobj() { }

		template<typename T>
		friend strobj&& operator<< (strobj&& out, const T& value)
		{
			out.ss << value;
			return std::move(out);
		}

		operator std::string() const { return ss.str(); }

	protected:
#if SLOG_NO_COPY == 1
		strobj(const strobj&);
		strobj& operator=(const strobj&);
#endif

		std::ostringstream ss;
};

#if SLOG_EXCEPTION_PRINT == 1
inline std::ostream& operator<< (std::ostream& out, const std::exception& e)
{
	out << e.what();
	return out;
}
#endif

namespace slog
{
	std::string getversion();
	uint32_t getmajorversion();
	uint32_t getminorversion();
	uint32_t getpatchversion();

	// a null logobj that all log object get replaced with when they are compiled out
	// see SLOG_DISABLE_*
	template<typename TYPE>
	class nooplogobj
	{
		public:
			nooplogobj() { }

			template<typename T>
			friend nooplogobj&& operator<< (nooplogobj&& out, const T& value)
			{
				return std::move(out);
			}

			operator std::string() const { return ""; }

			static TYPE type;

		protected:
#if SLOG_NO_COPY == 1
			nooplogobj(const nooplogobj&);
			nooplogobj& operator=(const nooplogobj&);
#endif
	};

	enum class consolecolor : uint8_t
	{
		White,
		Gray,
		Red,
		Green,
		Blue,
		Yellow,
		Magenta,
		Cyan,
	};

	struct logtype
	{
		logtype() : name("unnamed"), enabled(true), priority(0), tag(0), color(consolecolor::Gray) { }

		logtype(const char* _name, uint32_t _prio, uint32_t _tag, consolecolor _color) : enabled(true), name(_name), priority(_prio), tag(_tag), color(_color) {}

		bool usestderr;
		uint32_t tag;
		uint32_t priority;
		const char* name;
		bool enabled;
		consolecolor color;
	};

	class logconfig
	{
		public:
			typedef std::function<void(const logtype& ltype, const std::string& msg)> print_func;

			static void parse(int argc, char* argv[]);

			static std::string formatmsg(const logtype& ltype, const std::string& msg);

			static void default_console_print(const logtype& type, const std::string& line);

			static void get_current_date_time(tm& timedate);

			static bool usecolor;
			static bool timestamps;
			static bool print_logtype;
			static bool print_priority;

			static std::map<std::string, print_func> print_functions;
	};

	inline std::ostream& operator<< (std::ostream& out, const logtype& lt)
	{
		if (logconfig::print_priority)
			out << lt.priority << "|";
		out << lt.name;
		return out;
	}

	//---------------------------------------------------------------------

	class logdevice
	{
		public:
			logdevice(std::string deviceName, logconfig::print_func printFunc) : m_deviceName(std::move(deviceName))
			{
				logconfig::print_functions[deviceName] = printFunc;
			}

			~logdevice()
			{
				logconfig::print_functions.erase(m_deviceName);
			}

		private:
			std::string m_deviceName;
	};

	class filelogdevice : slog::logdevice
	{
		public:
			filelogdevice(const std::string& filename, bool bAppend = false);

			void writefile(const slog::logtype& type, const std::string& line);

		private:
			std::ofstream m_file;
	};
	
	//---------------------------------------------------------------------

	template<typename TYPE>
	class logobj
	{
		public:
			logobj() { }

			~logobj()
			{
				if (type.enabled)
				{
					std::string line = logconfig::formatmsg(type, ss.str());

					for (auto& each : logconfig::print_functions)
						if (each.second)
							each.second(type, line);
				}
			}

			template<typename T>
			friend logobj&& operator<< (logobj&& out, const T& value)
			{
				if (type.enabled)
					out.ss << value;

				return std::move(out);
			}

			static TYPE type;

		protected:
			std::ostringstream ss;
		
#if SLOG_NO_COPY == 1
		private:
			logobj(const logobj&);
			logobj& operator=(const logobj&);
#endif
	};

	// --------------------------------------------

	struct logtype_info : logtype
	{
		static const uint32_t Tag = 0x696e666f;

		logtype_info()
		{
			name = "info";
			priority = 100;
			tag = Tag;
			color = consolecolor::White;
		}
	};

	struct logtype_warn : logtype
	{
		static const uint32_t Tag = 0x7761726e;

		logtype_warn()
		{
			name = "warn";
			priority = 150;
			tag = Tag;
			color = consolecolor::Yellow;
		}
	};

	struct logtype_error : logtype
	{
		static const uint32_t Tag = 0x6572726f;

		logtype_error()
		{
			name = "errr";
			priority = 200;
			tag = Tag;
			color = consolecolor::Red;
		}
	};

	struct logtype_verbose : logtype
	{
		static const uint32_t Tag = 0x76657262;

		logtype_verbose()
		{
			enabled = false;
			name = "verb";
			priority = 50;
			tag = Tag;
			color = consolecolor::Cyan;
		}
	};

	struct logtype_debug : logtype
	{
		static const uint32_t Tag = 0x64656267;

		logtype_debug()
		{
			enabled = false;
			name = "debg";
			priority = 50;
			tag = Tag;
			color = consolecolor::Gray;
		}
	};

	// --------------------------------------------

#if SLOG_DISABLE != 1
#define curlogobj logobj
#else
#define curlogobj nooplogobj
#endif

#if SLOG_DISABLE_INFO != 1
#define curlogobj_info curlogobj
#else
#define curlogobj_info nooplogobj
#endif

#if SLOG_DISABLE_WARN != 1
#define curlogobj_warn curlogobj
#else
#define curlogobj_warn nooplogobj
#endif

#if SLOG_DISABLE_VERBOSE != 1
#define curlogobj_verbose curlogobj
#else
#define curlogobj_verbose nooplogobj
#endif

#if SLOG_DISABLE_ERROR != 1
#define curlogobj_error curlogobj
#else
#define curlogobj_error nooplogobj
#endif

#if SLOG_DISABLE_DEBUG != 1
#define curlogobj_debug curlogobj
#else
#define curlogobj_debug nooplogobj
#endif

	typedef curlogobj_info<logtype_info> info;
	typedef curlogobj_verbose<logtype_verbose> verbose;
	typedef curlogobj_warn<logtype_warn> warn;
	typedef curlogobj_error<logtype_error> error;
	typedef curlogobj_debug<logtype_debug> debug;
}; // namespace