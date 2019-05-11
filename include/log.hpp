#ifndef LOG_HPP
#define LOG_HPP

#include<stdio.h>
#include<string_view>

int initLog();
void log(std::string_view, ...);
int stopLog();

#endif // LOG_HPP

