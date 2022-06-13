
#include "os.h"

#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
namespace osutil{
    std::string getExePath(){
        char result[ PATH_MAX ];
        ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
        const char *path;
        if (count != -1) {
            path = dirname(result);
            return std::string( path );
        }
        return "";        
        // return std::string( result, (count > 0) ? count : 0 );
    }

    std::tm  now(){
        std::time_t ts = std::time(NULL);
        return *std::localtime(&ts);
    }
}