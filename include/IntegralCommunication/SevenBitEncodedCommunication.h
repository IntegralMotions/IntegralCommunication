#pragma once

#include <cstddef>
#include <cstdint>

#include "Communication.h"
#include <Diagnostics/Logger.h>

/**
 * @brief Communication wrapper that frames messages with 7-bit encoding.
 *
 * SevenBitEncodedCommunication writes each message as a 7-bit-safe byte stream.
 * Reads collect bytes until the encoding marks the end of a frame, decode one
 * complete message, and keep any later bytes buffered for the next readMessage
 * call.
 */
class SevenBitEncodedCommunication {
  public:
    /**
     * @brief Creates a 7-bit-framed communication wrapper.
     *
     * @param inner Communication instance used for the underlying transport.
     * @param txSize Number of bytes allocated for encoded outgoing frames.
     * @param rxSize Number of bytes allocated for encoded incoming frames.
     */
    SevenBitEncodedCommunication(Communication& inner, size_t txSize, size_t rxSize);
    /** The non-owning logger must outlive this wrapper. */
    SevenBitEncodedCommunication(Communication& inner, size_t txSize, size_t rxSize,
                                 IntegralMotions::Diagnostics::Logger& logger);

    /**
     * @brief Releases the internal transmit and receive buffers.
     */
    ~SevenBitEncodedCommunication();

    SevenBitEncodedCommunication(const SevenBitEncodedCommunication&) = delete;
    SevenBitEncodedCommunication& operator=(const SevenBitEncodedCommunication&) = delete;

    /**
     * @brief Moves the owned buffers from another 7-bit communication wrapper.
     *
     * @param other Wrapper to move from.
     */
    SevenBitEncodedCommunication(SevenBitEncodedCommunication&& other) noexcept;

    /**
     * @brief Moves the owned buffers from another 7-bit communication wrapper.
     *
     * @param other Wrapper to move from.
     * @return Reference to this wrapper.
     */
    SevenBitEncodedCommunication& operator=(SevenBitEncodedCommunication&& other) noexcept;

    /**
     * @brief Encodes and writes one complete 7-bit-framed message.
     *
     * @param data Payload bytes to send.
     * @param length Number of payload bytes to send.
     * @return Whether the message fit the transmit buffer and was written.
     */
    bool writeMessage(const uint8_t* data, size_t length);

    /**
     * @brief Reads and decodes one complete 7-bit-framed message.
     *
     * This function is non-blocking. It returns false when no complete frame is
     * currently available or when the next available frame cannot be decoded.
     *
     * @param out Buffer that receives decoded payload bytes.
     * @param maxOutLen Capacity of out in bytes.
     * @param outLen Receives the number of decoded payload bytes.
     * @return Whether a full frame was decoded.
     */
    bool readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen);

  private:
    Communication& _inner;

    uint8_t* _txBuffer;
    uint8_t* _rxBuffer;
    size_t _txSize;
    size_t _rxSize;
    size_t _rxIndex;
    bool _rxFullReported = false;
    IntegralMotions::Diagnostics::Logger* _logger = nullptr;
};
