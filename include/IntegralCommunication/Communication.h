#pragma once

#include <cstddef>
#include <cstdint>

class BufferedCommunication;

/**
 * @brief Abstract byte-oriented communication interface.
 *
 * Communication exposes a small public API for writing bytes, checking how
 * many bytes can be read, and reading bytes from an underlying transport.
 * Implementations provide the transport-specific behavior by overriding the
 * private implementation functions.
 */
class Communication {
  public:
    /**
     * @brief Creates a communication interface.
     */
    Communication() = default;

    /**
     * @brief Destroys a communication interface.
     */
    virtual ~Communication() = default;

    friend BufferedCommunication;

    /**
     * @brief Writes bytes to the underlying transport.
     *
     * @param data Buffer containing bytes to write.
     * @param size Number of bytes to write.
     */
    virtual void write(const uint8_t* data, size_t size);

    /**
     * @brief Returns the number of bytes available to read.
     *
     * @return Number of readable bytes currently available.
     */
    virtual size_t available();

    /**
     * @brief Reads bytes from the underlying transport.
     *
     * @param data Buffer that receives read bytes.
     * @param size Maximum number of bytes to read.
     * @return Number of bytes actually read.
     */
    virtual size_t read(uint8_t* data, size_t size);

  private:
    /**
     * @brief Transport-specific write implementation.
     *
     * @param data Buffer containing bytes to write.
     * @param size Number of bytes to write.
     * @return Number of bytes accepted by the transport.
     */
    virtual size_t writeImpl(const uint8_t* data, size_t size) = 0;

    /**
     * @brief Transport-specific available-byte query.
     *
     * @return Number of readable bytes currently available.
     */
    virtual size_t availableImpl() = 0;

    /**
     * @brief Transport-specific read implementation.
     *
     * @param data Buffer that receives read bytes.
     * @param size Maximum number of bytes to read.
     * @return Number of bytes actually read.
     */
    virtual size_t readImpl(uint8_t* data, size_t size) = 0;
};
