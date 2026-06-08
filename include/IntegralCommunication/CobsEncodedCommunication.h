#pragma once

#include <cstddef>
#include <cstdint>

#include "Communication.h"

/**
 * @brief Communication wrapper that frames messages with COBS encoding.
 *
 * CobsEncodedCommunication writes each message as a COBS-encoded payload
 * followed by the COBS delimiter byte. Reads collect bytes until the delimiter
 * is found, decode one complete frame, and keep any later bytes buffered for
 * the next readMessage call.
 */
class CobsEncodedCommunication {
  public:
    /**
     * @brief Creates a COBS-framed communication wrapper.
     *
     * @param inner Communication instance used for the underlying transport.
     * @param txSize Number of bytes allocated for encoded outgoing frames.
     * @param rxSize Number of bytes allocated for encoded incoming frames.
     */
    CobsEncodedCommunication(Communication& inner, size_t txSize, size_t rxSize);

    /**
     * @brief Releases the internal transmit and receive buffers.
     */
    ~CobsEncodedCommunication();

    CobsEncodedCommunication(const CobsEncodedCommunication&) = delete;
    CobsEncodedCommunication& operator=(const CobsEncodedCommunication&) = delete;

    /**
     * @brief Moves the owned buffers from another COBS communication wrapper.
     *
     * @param other Wrapper to move from.
     */
    CobsEncodedCommunication(CobsEncodedCommunication&& other) noexcept;

    /**
     * @brief Moves the owned buffers from another COBS communication wrapper.
     *
     * @param other Wrapper to move from.
     * @return Reference to this wrapper.
     */
    CobsEncodedCommunication& operator=(CobsEncodedCommunication&& other) noexcept;

    /**
     * @brief Encodes and writes one complete COBS-framed message.
     *
     * The frame delimiter is appended automatically after the encoded payload.
     *
     * @param data Payload bytes to send. May be nullptr when length is 0.
     * @param length Number of payload bytes to send.
     * @return Whether the message fit the transmit buffer and was written.
     */
    bool writeMessage(const uint8_t* data, size_t length);

    /**
     * @brief Reads and decodes one complete COBS-framed message.
     *
     * This function is non-blocking. It returns false when no complete frame is
     * currently available or when the next available frame is malformed.
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
};
