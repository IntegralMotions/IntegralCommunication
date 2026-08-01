#include "IntegralCommunication/Encoding/SevenBitEncoding.h"
#include <cstddef>
#include <cstdint>

namespace SevenBitEncoding {
    inline constexpr uint8_t LastSevenBits = 0x7F;
    inline constexpr uint8_t FirstBit = 0x80;
    inline constexpr int EncodingSize = 7;
    inline constexpr int MaxShiftsForValue = 32;

    size_t getEncodedSize(uint32_t value) {
        size_t size = 0;
        do {
            size++;
            value >>= EncodingSize;
        } while (value > 0);
        return size;
    }

    void encodeValue(uint32_t value, uint8_t* output) {
        size_t index = 0;
        do {
            uint8_t byte = value & LastSevenBits; // Take the lower 7 bits
            value >>= EncodingSize;
            if (value > 0) {
                byte |= FirstBit; // Set the 8th bit if more bytes are needed
            }
            output[index++] = byte;
        } while (value > 0);
    }

    uint32_t decodeValue(const uint8_t* input, size_t inputSize, size_t& consumedBytes) {
        uint32_t length = 0;
        size_t shift = 0;
        consumedBytes = 0;

        for (size_t i = 0; i < inputSize; i++) {
            uint8_t byte = input[i];
            length |= (byte & LastSevenBits) << shift; // Extract 7 bits and shift into place
            consumedBytes++;

            if ((byte & FirstBit) == 0) // Stop if the continuation bit is not set
            {
                break;
            }

            shift += EncodingSize;
            if (shift >= MaxShiftsForValue) // Prevent overflow for invalid input
            {
                break;
            }
        }

        return length;
    }

    size_t getEncodedBufferSize(size_t bufferLength) {
        return (bufferLength > 0) ? bufferLength + ((bufferLength - 1) / EncodingSize) + 1 : 1;
    }

    size_t encodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer) {
        if (inputLength == 0) {
            return 0;
        }
        size_t outIndex = 0;
        uint8_t carry = 0;
        int carryBits = 0;
        for (size_t i = 0; i < inputLength; i++) {
            uint8_t current = inputBuffer[i];
            uint8_t septet = carry | (current >> (carryBits + 1));
            outputBuffer[outIndex++] = septet | FirstBit;
            carryBits++;
            carry = (current & leftMask(carryBits)) << (EncodingSize - carryBits);
            if (carryBits == EncodingSize) {
                outputBuffer[outIndex++] = carry | FirstBit;
                carry = 0;
                carryBits = 0;
            }
        }
        if (carryBits != 0) {
            uint8_t septet = carry;
            outputBuffer[outIndex++] = septet;
        }
        outputBuffer[outIndex - 1] &= LastSevenBits;
        return outIndex;
    }

    size_t decodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer, size_t outputLength) {
        if (inputBuffer == nullptr || outputLength == 0) {
            return 0;
        }
        size_t decoded = 0;
        size_t encodedIndex = 0;
        int bitShiftIndex = 0;
        while (decoded < outputLength && encodedIndex + 1 < inputLength) {
            uint8_t currentByte = inputBuffer[encodedIndex] & LastSevenBits;
            uint8_t nextByte = inputBuffer[encodedIndex + 1] & LastSevenBits;
            int bits = bitShiftIndex + 1;
            uint8_t carry = (nextByte >> (EncodingSize - bits)) & ((1 << bits) - 1);
            uint8_t upperPart = currentByte & ((1 << (EncodingSize - bitShiftIndex)) - 1);
            uint8_t value = (upperPart << bits) | carry;
            outputBuffer[decoded++] = value;
            bitShiftIndex++;
            encodedIndex++;
            if (bitShiftIndex == EncodingSize) {
                encodedIndex++;
                bitShiftIndex = 0;
            }
        }
        return decoded;
    }

    bool isLastByte(uint8_t byte) {
        return (byte & FirstBit) == 0;
    }
} // namespace SevenBitEncoding
