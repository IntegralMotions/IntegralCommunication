#include <cstddef>
#include <cstdint>
#include <span>

struct Message {
    std::span<uint8_t> buffer;
    uint16_t crc;
};
