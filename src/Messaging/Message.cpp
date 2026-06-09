#include "IntegralCommunication/Messaging/Message.h"
#include "IntegralCommunication/CRC.h"

Message createMessage(std::span<uint8_t> data, bool calculateCrc) {
    return Message{.buffer = data, .crc = calculateCrc ? CRC::calculate(data.data(), data.size()) : uint16_t{0}};
}
