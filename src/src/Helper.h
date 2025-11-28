#ifndef SYSTEMMONITOR_HELPER_H
#define SYSTEMMONITOR_HELPER_H

#include <string.h>
#include <iostream>
class Helper {
public:
    static int string_to_int(std::string);
    static std::string int_to_string(int);
};

#endif //SYSTEMMONITOR_HELPER_H
