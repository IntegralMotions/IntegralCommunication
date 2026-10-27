#include "IntegralCommunication/Messaging/Message.h"
#include "Math/Crc16.h"

Message createMessage(std::span<uint8_t> data, bool calculateCrc) {
    return Message{.buffer = data,
                   .crc =
                       calculateCrc ? IntegralMotions::Math::Crc16::calculate(data.data(), data.size()) : uint16_t{0}};
}