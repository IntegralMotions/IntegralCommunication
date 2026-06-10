
# IntegralCommunication

Communication utilities for embedded C++ projects. The library provides base communication abstractions, encoded communication helpers, message payload metadata, and CRC-16/CCITT-FALSE checksum support.

## Requirements

- C++20
- CMake 3.16 or newer

## Install Using FetchContent

``` CMAKE
include(FetchContent)

FetchContent_Declare(
    IntegralCommunication
    GIT_REPOSITORY https://github.com/IntegralMotions/IntegralCommunication.git
    GIT_TAG        v1.0.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(IntegralCommunication)

target_link_libraries(MyApp PRIVATE IntegralCommunication::IntegralCommunication)
```

## Communication Classes

`Communication` is the byte-oriented base interface. Implement `writeImpl`, `availableImpl`, and `readImpl` for your transport, then use the public `write`, `available`, and `read` API in protocol code.

```cpp
#include "IntegralCommunication/Communication.h"

class SerialCommunication : public Communication {
  private:
    size_t writeImpl(const uint8_t* data, size_t size) override {
        // Write bytes to the transport.
        return size;
    }

    size_t availableImpl() override {
        // Return the number of readable bytes.
        return 0;
    }

    size_t readImpl(uint8_t* data, size_t size) override {
        // Read bytes from the transport.
        return 0;
    }
};
```

`BufferedCommunication` adds a buffered write path using storage supplied by the caller. It is useful when small writes should be grouped before reaching the underlying transport.

For framed messages, wrap a `Communication` instance with one of the encoded communication classes:

- `CobsEncodedCommunication` writes and reads COBS-delimited message frames.
- `SevenBitEncodedCommunication` writes and reads 7-bit-safe message frames.

Both wrappers expose `writeMessage(const uint8_t* data, size_t length)` and non-blocking `readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen)`.

## Messages And CRC

`Message` is a non-owning view over payload bytes plus a CRC field. Use `createMessage` to create a message from existing data and optionally calculate the CRC-16/CCITT-FALSE checksum.

```cpp
#include "IntegralCommunication/Messaging/Message.h"

#include <array>
#include <cstdint>

std::array<uint8_t, 3> payload = {0x01, 0x02, 0x03};

Message message = createMessage(payload);
Message messageWithoutCrc = createMessage(payload, false);
```

The returned `Message::buffer` is a `std::span<uint8_t>` and does not own the payload storage. Keep the original data alive while using the message.

You can also use the CRC helpers directly:

```cpp
#include "IntegralCommunication/CRC.h"

#include <array>
#include <cstdint>

std::array<uint8_t, 9> data = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

uint16_t crc = CRC::calculate(data.data(), data.size()); // 0x29B1
bool valid = CRC::validate(data.data(), data.size(), crc);
```

## License

Apache License 2.0
