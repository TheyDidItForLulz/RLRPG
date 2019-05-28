#include<utils.hpp>
#include<thread>

void sleep(double sec) {
    std::this_thread::sleep_for(std::chrono::duration<double>(sec));
}

