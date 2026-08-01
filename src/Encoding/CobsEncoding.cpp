#include "IntegralCommunication/Encoding/CobsEncoding.h"

namespace CobsEncoding {
    size_t getEncodedBufferSize(size_t inputLength) {
        if (inputLength == 0) {
            return 1;
        }
        return inputLength + ((inputLength + MaxBlockPayloadSize - 1) / MaxBlockPayloadSize);
    }

    size_t encodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer) {
        if (outputBuffer == nullptr || (inputBuffer == nullptr && inputLength > 0)) {
            return 0;
        }

        if (inputLength == 0) {
            outputBuffer[0] = 1;
            return 1;
        }

        size_t encodeIndex = 1;
        size_t codeIndex = 0;
        uint8_t code = 1;

        for (size_t i = 0; i < inputLength; i++) {
            if (inputBuffer[i] != Delimiter) {
                outputBuffer[encodeIndex++] = inputBuffer[i];
                ++code;
            }

            if (inputBuffer[i] == Delimiter || code == FullBlockCode) {
                outputBuffer[codeIndex] = code;
                code = 1;
                codeIndex = encodeIndex;

                if (inputBuffer[i] == Delimiter || i + 1 < inputLength) {
                    ++encodeIndex;
                }
            }
        }

        outputBuffer[codeIndex] = code;
        return encodeIndex;
    }

    bool decodeBuffer(const uint8_t* inputBuffer, size_t inputLength, uint8_t* outputBuffer, size_t outputLength,
                      size_t& decodedLength) {
        decodedLength = 0;

        if (inputBuffer == nullptr || inputLength == 0) {
            return false;
        }

        size_t outputIndex = 0;
        uint8_t code = FullBlockCode;
        uint8_t block = 0;

        for (size_t inputIndex = 0; inputIndex < inputLength;) {
            if (block != 0) {
                if (inputBuffer[inputIndex] == Delimiter || outputBuffer == nullptr || outputIndex >= outputLength) {
                    return false;
                }
                outputBuffer[outputIndex++] = inputBuffer[inputIndex++];
                --block;
            } else {
                block = inputBuffer[inputIndex++];
                if (block == Delimiter) {
                    return false;
                }

                if (code != FullBlockCode) {
                    if (outputBuffer == nullptr || outputIndex >= outputLength) {
                        return false;
                    }
                    outputBuffer[outputIndex++] = Delimiter;
                }

                code = block;
                --block;
            }
        }

        if (block != 0) {
            return false;
        }

        decodedLength = outputIndex;
        return true;
    }

    bool isDelimiter(uint8_t byte) {
        return byte == Delimiter;
    }
} // namespace CobsEncoding
