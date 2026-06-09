

#include <cstddef>
#include <cstdint>
namespace CRC {
    uint16_t calculate(const uint8_t* buffer, size_t bufferSize);
    bool validate(const uint8_t* buffer, size_t bufferSize, uint16_t crc);
} // namespace CRC