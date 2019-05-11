#ifndef LOG_HPP
#define LOG_HPP

#include<string_view>
#include<fstream>
#include<fmt/core.h>

namespace detail {
    void initLog();
}

template<class ... Args>
void log(std::string_view fmtstring, Args && ... args) {
    extern std::ofstream logfile;
    if (not logfile) {
        detail::initLog();
    }
    logfile << fmt::format(fmtstring, std::forward<Args>(args)...) << '\n';
}

#endif // LOG_HPP

