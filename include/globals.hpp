#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include<string>
#include<termlib/termlib.hpp>

#include<unit.hpp>

extern std::string message;
extern bool g_stop;
extern int g_turns;
extern int g_mode;
extern TerminalRenderer termRend;
extern TerminalReader termRead;
extern Hero * g_hero;

#endif // GLOBALS_HPP

