#include <iostream>

#include "PacketDecoder.h"
#include "NumberConvertor.h"

int main()
{
    std::ifstream ifs("input.txt");
    PacketDecoder packetDecoder(ifs);
    packetDecoder.decode();
    return 0;
}