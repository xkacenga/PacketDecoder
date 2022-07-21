#include "PacketDecoder.h"
#include "NumberConvertor.h"

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

PacketDecoder::PacketDecoder(std::ifstream &fs) {
    char c;
    while (fs.get(c)) {
        ss << NumberConvertor::convertHexToBinary(c);
    }
}

void PacketDecoder::decode() {
    PacketResult result = readPacket();
    std::cout << "value: " << result.value << "\n";
    std::cout << "versionSum: " << versionSum << "\n";
}

PacketResult PacketDecoder::readPacket() {
    Header header = readHeader();
    PacketResult result = {VERSION_SIZE + TYPE_ID_SIZE, 0};
    versionSum += header.version;

    PacketResult subResult;
    if (header.typeId == 4) {
        subResult = readLiteralPacket();
    } else {
        subResult = readOperatorPacket(header.typeId);
    }
    result.packetLength += subResult.packetLength;
    result.value = subResult.value;
    return result;
}

PacketResult PacketDecoder::readLiteralPacket() {
    PacketResult literalResult = readLiteralValue();
    int extraLength = removeExtraBits();
    return {literalResult.packetLength + extraLength, literalResult.value};
}

PacketResult PacketDecoder::readLiteralValue() {
    std::stringstream literal;
    int literalLength = 0;

    std::string bitGroup(LITERAL_GROUP_SIZE, ' ');
    do {
        ss.read(&bitGroup[0], LITERAL_GROUP_SIZE);
        literal << bitGroup.substr(1);
        literalLength += LITERAL_GROUP_SIZE;
    } while (bitGroup[0] != '0');
    return {literalLength, NumberConvertor::convertBinaryToLong(literal.str())};
}

int PacketDecoder::removeExtraBits() {
    int extraLength = ss.str().length() % 4;
    std::string extra(extraLength, ' ');
    ss.read(&extra[0], extraLength);
    return extraLength;
}

PacketResult PacketDecoder::readOperatorPacket(int typeId) {
    int lengthTypeId = readBits(LENGTH_TYPE_ID_SIZE);
    PacketResult result = {LENGTH_TYPE_ID_SIZE, 0};
    std::vector<long> subOperationsResults;
    if (lengthTypeId == 0) {
        int lengthOfSubpackets = readBits(SUBPACKET_LENGTH_SIZE);
        result.packetLength += SUBPACKET_LENGTH_SIZE;
        while (lengthOfSubpackets > 0) {
            PacketResult subResult = readPacket();
            subOperationsResults.push_back(subResult.value);
            result.packetLength += subResult.packetLength;
            lengthOfSubpackets -= subResult.packetLength;
        }
    } else {
        int numberOfSubpackets = readBits(NUMBER_OF_SUBPACKETS_SIZE);
        result.packetLength += NUMBER_OF_SUBPACKETS_SIZE;
        for (int i = 0; i < numberOfSubpackets; i++) {
            PacketResult subResult = readPacket();
            subOperationsResults.push_back(subResult.value);
            result.packetLength += subResult.packetLength;
        }
    }
    result.value = calculateOperationResult(typeId, subOperationsResults);
    return result;
}

long PacketDecoder::calculateOperationResult(int typeId, const std::vector<long> &results) {
    switch(typeId) {
        case 0: return std::accumulate(results.begin(), results.end(), 0L, std::plus<long>());
        case 1: return std::accumulate(results.begin(), results.end(), 1L, std::multiplies<long>());
        case 2: return *std::min_element(results.begin(), results.end());
        case 3: return *std::max_element(results.begin(), results.end());
        case 5: return results[0] > results[1] ? 1 : 0;
        case 6: return results[0] < results[1] ? 1 : 0;
        case 7: return results[0] == results[1] ? 1 : 0;
        default: return 0;
    }
}

Header PacketDecoder::readHeader() {
    long version = readBits(VERSION_SIZE);
    long typeId = readBits(TYPE_ID_SIZE);
    return Header{version, typeId};
}

long PacketDecoder::readBits(int count) {
    std::string bits;
    bits.reserve(count);
    ss.read(&bits[0], count);
    return NumberConvertor::convertBinaryToLong(bits);
}