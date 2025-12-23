#pragma once

#include "IntegralCommunication/Communication.h"
#include <cstddef>
#include <cstdint>

class BufferedCommunication : public Communication {
  public:
    BufferedCommunication(uint8_t* buffer, size_t bufferSize);
    virtual ~BufferedCommunication();

    void write(const uint8_t* data, size_t dataSize);
    void flush();
    size_t available();
    size_t read(uint8_t* data, size_t dataSize);

  protected:
    [[nodiscard]] uint8_t* buffer() noexcept;
    [[nodiscard]] const uint8_t* buffer() const noexcept;
    [[nodiscard]] size_t bufferIndex() const noexcept;
    [[nodiscard]] size_t bufferSize() const noexcept;

  private:
    uint8_t* _buffer;
    size_t _bufferSize;
    size_t _bufferIndex;
};
