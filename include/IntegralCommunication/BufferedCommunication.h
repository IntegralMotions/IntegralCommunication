#pragma once

#include "IntegralCommunication/Communication.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief Communication base class with a buffered write path.
 *
 * BufferedCommunication stores outgoing bytes in a caller-provided buffer until
 * the buffer is full or flush() is called. Derived classes implement the
 * underlying transport by overriding Communication's implementation functions.
 */
class BufferedCommunication : public Communication {
  public:
    /**
     * @brief Creates a buffered communication wrapper.
     *
     * @param buffer Storage used for pending outgoing bytes.
     * @param bufferSize Capacity of buffer in bytes.
     */
    BufferedCommunication(uint8_t* buffer, size_t bufferSize);

    /**
     * @brief Flushes any pending bytes and destroys the buffered wrapper.
     */
    virtual ~BufferedCommunication();

    /**
     * @brief Buffers bytes for transmission.
     *
     * Data is flushed to the underlying write implementation when the internal
     * buffer becomes full.
     *
     * @param data Buffer containing bytes to write.
     * @param dataSize Number of bytes to write.
     */
    void write(const uint8_t* data, size_t dataSize);

    /**
     * @brief Writes all currently buffered bytes to the underlying transport.
     */
    void flush();

    /**
     * @brief Returns the number of bytes available to read from the transport.
     *
     * @return Number of readable bytes currently available.
     */
    size_t available();

    /**
     * @brief Reads bytes from the underlying transport.
     *
     * @param data Buffer that receives read bytes.
     * @param dataSize Maximum number of bytes to read.
     * @return Number of bytes actually read.
     */
    size_t read(uint8_t* data, size_t dataSize);

  protected:
    /**
     * @brief Returns the internal write buffer.
     *
     * @return Mutable pointer to the internal write buffer.
     */
    [[nodiscard]] uint8_t* buffer() noexcept;

    /**
     * @brief Returns the internal write buffer.
     *
     * @return Const pointer to the internal write buffer.
     */
    [[nodiscard]] const uint8_t* buffer() const noexcept;

    /**
     * @brief Returns the number of bytes currently buffered.
     *
     * @return Current write buffer index.
     */
    [[nodiscard]] size_t bufferIndex() const noexcept;

    /**
     * @brief Returns the write buffer capacity.
     *
     * @return Buffer capacity in bytes.
     */
    [[nodiscard]] size_t bufferSize() const noexcept;

  private:
    uint8_t* _buffer;
    size_t _bufferSize;
    size_t _bufferIndex;
};
