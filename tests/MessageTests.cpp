#include "IntegralCommunication/Messaging/Message.h"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>

TEST(MessageTests, CreateMessageCalculatesCrc) {
    std::array<uint8_t, 9> data = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    const Message message = createMessage(data);

    EXPECT_EQ(message.buffer.data(), data.data());
    EXPECT_EQ(message.buffer.size(), data.size());
    EXPECT_EQ(message.crc, 0x29B1);
}

TEST(MessageTests, CreateMessageKeepsBufferNonOwning) {
    std::array<uint8_t, 4> data = {0x12, 0x34, 0x56, 0x78};

    Message message = createMessage(data);
    data[0] = 0xAB;

    EXPECT_EQ(message.buffer[0], 0xAB);
}

TEST(MessageTests, CreateMessageCanSkipCrcCalculation) {
    std::array<uint8_t, 9> data = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    const Message message = createMessage(data, false);

    EXPECT_EQ(message.buffer.data(), data.data());
    EXPECT_EQ(message.buffer.size(), data.size());
    EXPECT_EQ(message.crc, 0);
}
