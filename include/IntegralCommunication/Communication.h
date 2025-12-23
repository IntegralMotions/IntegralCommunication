#pragma once

#include <cstddef>
#include <cstdint>

class BufferedCommunication;

class Communication {
  public:
    Communication() = default;
    virtual ~Communication() = default;
    friend BufferedCommunication;

    void write(const uint8_t* data, size_t size);
    size_t available();
    size_t read(uint8_t* data, size_t size);

  private:
    virtual size_t writeImpl(const uint8_t* data, size_t size) = 0;
    virtual size_t availableImpl() = 0;
    virtual size_t readImpl(uint8_t* data, size_t size) = 0;
};
