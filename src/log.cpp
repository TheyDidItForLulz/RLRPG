#include"log.hpp"

std::ofstream logfile;

void detail::initLog() {
    logfile.open("log.txt");
}

