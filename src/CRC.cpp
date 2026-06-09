#include "IntegralCommunication/CRC.h"
#include <cstdint>

namespace CRC {
    inline constexpr uint16_t InitialValue = 0xffff;
    inline constexpr uint8_t BitsPerByte = 8;
    inline constexpr uint16_t MostSignificantBitMask = 0x8000;

    uint16_t calculate(const uint8_t* buffer, size_t bufferSize) {
        constexpr uint16_t polynomial = 0x1021;
        uint16_t crc = InitialValue;

        for (size_t i = 0; i < bufferSize; ++i) {
            crc ^= static_cast<uint16_t>(buffer[i]) << BitsPerByte;

            for (uint8_t bit = 0; bit < BitsPerByte; ++bit) {
                if ((crc & MostSignificantBitMask) != 0) {
                    crc = static_cast<uint16_t>((crc << 1) ^ polynomial);
                } else {
                    crc = static_cast<uint16_t>(crc << 1);
                }
            }
        }

        return crc;
    }

    bool validate(const uint8_t* buffer, size_t bufferSize, uint16_t crc) {
        return calculate(buffer, bufferSize) == crc;
    }
} // namespace CRC
