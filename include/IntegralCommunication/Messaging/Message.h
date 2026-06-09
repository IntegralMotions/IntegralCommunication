#include <cstddef>
#include <cstdint>
#include <span>

/**
 * @brief Payload bytes and their CRC checksum.
 */
struct Message {
    /**
     * @brief Message payload bytes.
     *
     * The span does not own the underlying storage.
     */
    std::span<uint8_t> buffer;

    /**
     * @brief CRC-16/CCITT-FALSE checksum calculated over buffer.
     */
    uint16_t crc;
};

/**
 * @brief Creates a message and optionally calculates its CRC-16/CCITT-FALSE checksum.
 *
 * The returned message references data; it does not copy or own the bytes.
 *
 * @param data Message payload bytes.
 * @param calculateCrc Whether to calculate the checksum. When false, crc is set to 0.
 */
[[nodiscard]] Message createMessage(std::span<uint8_t> data, bool calculateCrc = true);
