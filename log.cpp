#include"include/log.hpp"
#include<stdarg.h>
#include<string>

static FILE * logfile;

int initLog() {
	logfile = fopen("log.txt", "w");
	return logfile == nullptr;
}

void log(std::string_view fmt, ...) {
	if (logfile) {
		va_list vargs;
		va_start(vargs, fmt);
		vfprintf(logfile, std::string(fmt).c_str(), vargs);
		fflush(logfile);
		va_end(vargs);
	}
}

int stopLog() {
	if (logfile == nullptr) {
		return 1;
	}
	fclose(logfile);
}

