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
    std::stringstream literal;
    int literalLength = 0;

    std::string bitGroup(LITERAL_GROUP_SIZE, ' ');
    do {
        ss.read(&bitGroup[0], LITERAL_GROUP_SIZE);
        literal << bitGroup.substr(1);
        literalLength += LITERAL_GROUP_SIZE;
    } while (bitGroup[0] != '0');

    int extraLength = ss.str().length() % 4;
    std::string extra(extraLength, ' ');
    ss.read(&extra[0], extraLength);
    return {literalLength + extraLength, NumberConvertor::convertBinaryToLong(literal.str())};
}

PacketResult PacketDecoder::readOperatorPacket(int typeId) {
    char lengthTypeId = readLengthTypeId();
    PacketResult result = {LENGTH_TYPE_ID_SIZE, 0};
    std::vector<unsigned long> subOperationsResults;
    if (lengthTypeId == '0') {
        int lengthOfSubpackets = readLengthOfSubpackets();
        result.packetLength += SUBPACKET_LENGTH_SIZE;
        while (lengthOfSubpackets > 0) {
            PacketResult subResult = readPacket();
            subOperationsResults.push_back(subResult.value);
            result.packetLength += subResult.packetLength;
            lengthOfSubpackets -= subResult.packetLength;
        }
    } else {
        int numberOfSubpackets = readNumberOfSubpackets();
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

unsigned long PacketDecoder::calculateOperationResult(int typeId, const std::vector<unsigned long> &results) {
    switch(typeId) {
        case 0: return std::accumulate(results.begin(), results.end(), 0UL, std::plus<unsigned long>());
        case 1: return std::accumulate(results.begin(), results.end(), 1UL, std::multiplies<unsigned long>());
        case 2: return *std::min_element(results.begin(), results.end());
        case 3: return *std::max_element(results.begin(), results.end());
        case 5: return results[0] > results[1] ? 1 : 0;
        case 6: return results[0] < results[1] ? 1 : 0;
        case 7: return results[0] == results[1] ? 1 : 0;
        default: return 0;
    }
}

Header PacketDecoder::readHeader() {
    std::string version(VERSION_SIZE, ' ');
    ss.read(&version[0], VERSION_SIZE);
    std::string typeId(TYPE_ID_SIZE, ' ');
    ss.read(&typeId[0], TYPE_ID_SIZE);
    return Header{NumberConvertor::convertBinaryToLong(version), NumberConvertor::convertBinaryToLong(typeId)};
}

char PacketDecoder::readLengthTypeId() {
    char lengthTypeId;
    ss.read(&lengthTypeId, LENGTH_TYPE_ID_SIZE);
    return lengthTypeId;
}

int PacketDecoder::readLengthOfSubpackets() {
    std::string lengthOfSubPackets(SUBPACKET_LENGTH_SIZE, ' ');
    ss.read(&lengthOfSubPackets[0], SUBPACKET_LENGTH_SIZE);
    return NumberConvertor::convertBinaryToLong(lengthOfSubPackets);
}

int PacketDecoder::readNumberOfSubpackets() {
    std::string numberOfSubPackets(NUMBER_OF_SUBPACKETS_SIZE, ' ');
    ss.read(&numberOfSubPackets[0], NUMBER_OF_SUBPACKETS_SIZE);
    return NumberConvertor::convertBinaryToLong(numberOfSubPackets);
}