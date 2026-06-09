#include "IntegralCommunication/CRC.h"
#include <cstdint>
#include <gtest/gtest.h>

TEST(CRCTests, CalculatesCrc16CcittFalseCheckValue) {
    uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    EXPECT_EQ(CRC::calculate(data, sizeof(data)), 0x29B1);
}

TEST(CRCTests, CalculatesInitialValueForEmptyBuffer) {
    uint8_t data[] = {};

    EXPECT_EQ(CRC::calculate(data, 0), 0xFFFF);
}

TEST(CRCTests, ValidatesMatchingCrc) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    const uint16_t crc = CRC::calculate(data, sizeof(data));

    EXPECT_TRUE(CRC::validate(data, sizeof(data), crc));
    EXPECT_FALSE(CRC::validate(data, sizeof(data), static_cast<uint16_t>(crc ^ 0x0001)));
}
