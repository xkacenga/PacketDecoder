#ifndef NUMBER_CONVERTOR_H
#define NUMBER_CONVERTOR_H

#include <string>

class NumberConvertor {
    NumberConvertor() = delete;
public:
    static std::string convertHexToBinary(char hex);
    static unsigned long long convertBinaryToLong(const std::string &binary);
};

#endif