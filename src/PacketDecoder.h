#ifndef PACKET_DECODER_H
#define PACKET_DECODER_H

#include <sstream>
#include <fstream>
#include <vector>

struct Header {
    unsigned long version;
    unsigned long typeId;
};

struct PacketResult {
    int packetLength;
    unsigned long value;
};

class PacketDecoder {
    std::stringstream ss;
    int versionSum = 0;

    const int LITERAL_GROUP_SIZE = 5;
    const int VERSION_SIZE = 3;
    const int TYPE_ID_SIZE = 3;
    const int SUBPACKET_LENGTH_SIZE = 15;
    const int NUMBER_OF_SUBPACKETS_SIZE = 11;
    const int LENGTH_TYPE_ID_SIZE = 1;

    PacketResult readPacket();
    PacketResult readOperatorPacket(int typeId);
    PacketResult readLiteralPacket();
    Header readHeader();
    int readNumberOfSubpackets();
    int readLengthOfSubpackets();
    char readLengthTypeId();
    static unsigned long calculateOperationResult(int typeId, const std::vector<unsigned long> &results);
public:
    PacketDecoder(std::ifstream &fs);
    void decode();
};

#endif