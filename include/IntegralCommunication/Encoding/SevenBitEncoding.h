#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @namespace SevenBitEncoding
 * @brief Stateless utilities for 7-bit-safe byte encoding.
 *
 * SevenBitEncoding transforms arbitrary 8-bit binary data into bytes where the
 * most significant bit is used as a continuation marker. This is useful for
 * transports that need encoded message bytes to identify the end of a frame.
 */
namespace SevenBitEncoding {
    /**
     * @brief Returns the number of encoded bytes needed for a 32-bit value.
     *
     * @param value Value to measure.
     * @return Number of bytes needed to encode value.
     */
    size_t getEncodedSize(uint32_t value);

    /**
     * @brief Encodes a 32-bit value into a 7-bit variable-length representation.
     *
     * output must have at least getEncodedSize(value) bytes available.
     *
     * @param value Value to encode.
     * @param output Buffer that receives the encoded bytes.
     */
    void encodeValue(uint32_t value, uint8_t* output);

    /**
     * @brief Decodes a 7-bit variable-length value.
     *
     * @param input Buffer containing encoded bytes.
     * @param inputSize Number of bytes available in input.
     * @param consumedBytes Receives the number of bytes consumed from input.
     * @return Decoded value.
     */
    uint32_t decodeValue(const uint8_t* input, size_t inputSize, size_t& consumedBytes);

    /**
     * @brief Returns the maximum buffer size needed to encode input bytes.
     *
     * @param bufferLength Number of bytes in the unencoded input buffer.
     * @return Maximum encoded buffer size in bytes.
     */
    size_t getEncodedBufferSize(size_t bufferLength);

    /**
     * @brief Encodes an 8-bit byte buffer into 7-bit-safe bytes.
     *
     * outputBuffer must have at least getEncodedBufferSize(inputLength) bytes
     * available.
     *
     * @param inputBuffer Buffer containing bytes to encode.
     * @param inputLength Number of bytes to read from inputBuffer.
     * @param outputBuffer Buffer that receives encoded bytes.
     * @return Number of encoded bytes written.
     */
    size_t encodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer);

    /**
     * @brief Decodes a 7-bit-safe byte buffer back to 8-bit bytes.
     *
     * @param inputBuffer Buffer containing encoded bytes.
     * @param inputLength Number of bytes to read from inputBuffer.
     * @param outputBuffer Buffer that receives decoded bytes.
     * @param outputLength Capacity of outputBuffer in bytes.
     * @return Number of decoded bytes written.
     */
    size_t decodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer, size_t outputLength);

    /**
     * @brief Checks whether an encoded byte marks the end of an encoded sequence.
     *
     * @param byte Encoded byte to inspect.
     * @return Whether the byte has no continuation bit.
     */
    bool isLastByte(uint8_t byte);

    /**
     * @brief Creates a mask with the n least-significant bits set.
     *
     * @param n Number of low bits to set.
     * @return Bit mask with the n least-significant bits set.
     */
    inline uint8_t leftMask(uint8_t n) {
        return static_cast<uint8_t>((1 << (n)) - 1);
    }
} // namespace SevenBitEncoding
