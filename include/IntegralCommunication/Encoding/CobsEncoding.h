#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @namespace CobsEncoding
 * @brief Stateless utilities for Consistent Overhead Byte Stuffing (COBS).
 *
 * COBS transforms arbitrary binary data into a byte stream that never contains
 * 0x00. This is useful when 0x00 is used as a packet delimiter on a serial
 * transport. These functions encode and decode only the COBS payload; they do
 * not append or consume a delimiter byte.
 *
 * The API is a namespace instead of a class because COBS encoding is stateless.
 */
namespace CobsEncoding {
    /**
     * @brief Packet delimiter byte used with COBS-framed messages.
     */
    inline constexpr uint8_t Delimiter = 0x00;

    /**
     * @brief COBS code byte used for a full block.
     *
     * A value of 0xFF marks a block containing 254 non-zero payload bytes.
     */
    inline constexpr uint8_t FullBlockCode = 0xFF;

    /**
     * @brief Maximum number of non-zero payload bytes represented by one COBS code byte.
     */
    inline constexpr size_t MaxBlockPayloadSize = 254;

    /**
     * @brief Returns the maximum number of bytes needed to COBS-encode a buffer.
     *
     * The returned size does not include a trailing delimiter. For non-empty
     * input this is inputLength + ceil(inputLength / 254). Allocate one extra
     * byte if the encoded packet will be followed by CobsEncoding::Delimiter.
     *
     * @param inputLength Number of bytes in the input buffer.
     * @return Maximum encoded payload size in bytes, excluding a delimiter.
     */
    size_t getEncodedBufferSize(size_t inputLength);

    /**
     * @brief Encodes a binary buffer into COBS format.
     *
     * outputBuffer must have at least getEncodedBufferSize(inputLength) bytes
     * available. inputBuffer may be nullptr only when inputLength is 0.
     *
     * @param inputBuffer Buffer containing bytes to encode. May be nullptr when inputLength is 0.
     * @param inputLength Number of bytes to read from inputBuffer.
     * @param outputBuffer Buffer that receives the encoded payload.
     * @return Number of encoded bytes written, or 0 when the arguments are invalid.
     *
     * @note Encoding an empty input writes a single 0x01 byte and returns 1.
     */
    size_t encodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer);

    /**
     * @brief Decodes a COBS payload back to binary data.
     *
     * outputLength is the capacity of outputBuffer. decodedLength receives the
     * number of decoded bytes written. outputBuffer may be nullptr only when the
     * encoded payload decodes to zero bytes.
     *
     * @param inputBuffer Buffer containing the encoded COBS payload.
     * @param inputLength Number of bytes to read from inputBuffer.
     * @param outputBuffer Buffer that receives decoded bytes.
     * @param outputLength Capacity of outputBuffer in bytes.
     * @param decodedLength Receives the number of decoded bytes written.
     * @return Whether decoding succeeded.
     */
    bool decodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer, size_t outputLength,
                      size_t& decodedLength);

    /**
     * @brief Checks whether a byte is the COBS frame delimiter.
     *
     * @param byte Byte to inspect.
     * @return Whether byte equals CobsEncoding::Delimiter.
     */
    bool isDelimiter(uint8_t byte);
} // namespace CobsEncoding
