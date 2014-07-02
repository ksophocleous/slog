slog
==
a simple c++ logging library. The project is build using `cmake` and is meant to be used as a static library, though nothing prevents you from just including the files in your project directly.

```C++
#include <slog/slog.h>

using slog::info;
using slog::error;
using slog::warn;

int main(int argc, char* argv[])
{
    slog::logconfig global_log_config(argc, argv);
    
    info() << "this will print a line with timestamp and log level info.";
    error() << "and on supported terminals (xterm, win32 console) this line will be red";
    
    {
        slog::logdevice_file keep_this_instance_around("logoutput.log"); 
        warn() << "warning: this should print on console yellow but it will also appear in the log file";
    }
    
    return 0;
}
```