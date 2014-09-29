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
#include <iostream>
#include <sstream>
#include <map>
#include <stdexcept>

#ifndef SLOG_NO_COPY
#define SLOG_NO_COPY 1
#endif

#ifndef SLOG_EXCEPTION_PRINT 
#define SLOG_EXCEPTION_PRINT 1
#endif

#ifndef SLOG_STROBJ_NAMESPACE
#define SLOG_STROBJ_NAMESPACE 0
#endif

#if SLOG_STROBJ_NAMESPACE == 1
namespace slog
{
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

#if SLOG_STROBJ_NAMESPACE == 1
}
#endif

#define _stringify(_s) #_s
#define stringify(_s) _stringify(_s)

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

	template<typename TYPE> TYPE nooplogobj<TYPE>::type;

	enum class consolecolor : uint8_t
	{
		white,
		gray,
		red,
		green,
		blue,
		yellow,
		magenta,
		cyan,
	};

	struct logtype
	{
		logtype() : name("unnamed"), enabled(true), priority(0), tag(0), color(consolecolor::gray) { }

		logtype(const char* _name, uint32_t _prio, uint32_t _tag, consolecolor _color) : enabled(true), name(_name), priority(_prio), tag(_tag), color(_color) {}

		bool usestderr;
		uint32_t tag;
		uint32_t priority;
		std::string name;
		bool enabled;
		consolecolor color;
	};

	class logdevice;
	class logdevice_console;

	class logconfig
	{
		public:
			logconfig();
			logconfig(int argc, char* argv[]);
			~logconfig();

			void parse(int argc, char* argv[]);

			static std::string formatmsg(const logtype& ltype, const std::string& msg);

			bool usecolor;
			bool timestamps;
			bool print_logtype;
			bool print_priority;

			static std::map<std::string, logdevice*> print_functions;

			static const logconfig* _cur_config;

		private:
			const logconfig* _prev_config;
	};

#if SLOG_EXCEPTION_PRINT == 1
	inline std::ostream& operator<< (std::ostream& out, const logtype& lt)
	{
		if (logconfig::_cur_config->print_priority)
			out << lt.priority << "|";
		out << lt.name;
		return out;
	}
#endif

	//---------------------------------------------------------------------

	class logdevice
	{
		public:
			logdevice(std::string deviceName) : m_deviceName(std::move(deviceName))
			{
				_prev_device = logconfig::print_functions[m_deviceName];
				logconfig::print_functions[m_deviceName] = this;
			}

			~logdevice()
			{
				if (_prev_device)
					logconfig::print_functions[m_deviceName] = _prev_device;
				else
					logconfig::print_functions.erase(m_deviceName);
			}

			virtual void writelogline(const slog::logtype& type, const std::string& line) = 0;

		private:
			std::string m_deviceName;
			logdevice* _prev_device;
	};
	
	//---------------------------------------------------------------------

	template<typename TYPE>
	class logobj
	{
		public:
			logobj() { }

			~logobj()
			{
				try
				{
					if (type.enabled)
					{
						std::string line = logconfig::formatmsg(type, ss.str());

						for (auto& each : logconfig::print_functions)
							if (each.second)
								each.second->writelogline(type, line);
					}																			
				}
				catch (...)
				{
					std::cerr << "logobj caught an exception most likely thrown by a writelogline" << std::endl;
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

	inline std::ostream& operator<< (std::ostream& out, const std::exception& exception)
	{
		out << exception.what();
		return out;
	}

	template<typename TYPE> TYPE logobj<TYPE>::type;

	// --------------------------------------------

	struct logtype_info : logtype
	{
		static const uint32_t Tag = 0x696e666f;

		logtype_info()
		{
			name = "info";
			priority = 100;
			tag = Tag;
			color = consolecolor::white;
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
			color = consolecolor::yellow;
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
			usestderr = true;
			color = consolecolor::red;
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
			color = consolecolor::cyan;
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
			color = consolecolor::gray;
		}
	};

	struct logtype_success : logtype
	{
		static const uint32_t Tag = 0x73756363;

		logtype_success()
		{
			enabled = true;
			name = "succ";
			priority = 100;
			tag = Tag;
			color = consolecolor::green;
		}
	};

	// --------------------------------------------

#if SLOG_DISABLE_INFO != 1 && SLOG_DISABLE != 1
	typedef logobj<logtype_info> info;
#else
	typedef nooplogobj<logtype_info> info;
#endif

#if SLOG_DISABLE_WARN != 1 && SLOG_DISABLE != 1
	typedef logobj<logtype_warn> warn;
#else
	typedef nooplogobj<logtype_warn> warn;
#endif

#if SLOG_DISABLE_VERBOSE != 1 && SLOG_DISABLE != 1
	typedef logobj<logtype_verbose> verbose;
#else
	typedef nooplogobj<logtype_verbose> verbose;
#endif

#if SLOG_DISABLE_ERROR != 1 && SLOG_DISABLE != 1
	typedef logobj<logtype_error> error;
#else
	typedef nooplogobj<logtype_error> error;
#endif

#if SLOG_DISABLE_DEBUG != 1 && SLOG_DISABLE != 1
	typedef logobj<logtype_debug> debug;
#else
	typedef nooplogobj<logtype_debug> debug;
#endif

#if SLOG_DISABLE_SUCCESS != 1 && SLOG_DISABLE != 1
	typedef logobj<logtype_success> success;
#else
	typedef nooplogobj<logtype_success> success;
#endif

#ifndef BUILDING_SLOG
	extern template class logobj<logtype_info>;
	extern template class logobj<logtype_warn>;
	extern template class logobj<logtype_error>;
	extern template class logobj<logtype_verbose>;
	extern template class logobj<logtype_debug>;
	extern template class logobj<logtype_success>;
#endif

}; // namespace
