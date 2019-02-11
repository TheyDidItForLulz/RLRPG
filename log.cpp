#include"include/log.hpp"
#include<stdarg.h>

static FILE * logfile;

int initLog() {
	logfile = fopen("log.txt", "w");
	return logfile == nullptr;
}

void log(const char * fmt, ...) {
	if (logfile) {
		va_list vargs;
		va_start(vargs, fmt);
		vfprintf(logfile, fmt, vargs);
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

