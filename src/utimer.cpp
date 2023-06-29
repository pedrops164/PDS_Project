#include "utimer.h"

utimer::utimer(const std::string m) : message(m) {
    start = std::chrono::system_clock::now();
    runs = 1;
}

utimer::utimer(const std::string m, int runs) : message(m), runs(runs) {
    start = std::chrono::system_clock::now();
}
utimer::~utimer() {
    stop =
        std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed =
        (stop - start) / runs; //divide the number of runs to get an average
    auto musec =
        std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout << message << " computed in " << std::setw(15) << musec << " usec "
        << std::endl;
}