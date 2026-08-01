#include "IntegralCommunication/SevenBitEncodedCommunication.h"
#include "IntegralCommunication/Encoding/SevenBitEncoding.h"
#include <algorithm>
#include <cstring>

SevenBitEncodedCommunication::SevenBitEncodedCommunication(Communication& inner, size_t txSize, size_t rxSize)
    : _inner(inner), _txBuffer((txSize != 0U) ? new uint8_t[txSize] : nullptr),
      _rxBuffer((rxSize != 0U) ? new uint8_t[rxSize] : nullptr), _txSize(txSize), _rxSize(rxSize), _rxIndex(0) {}

SevenBitEncodedCommunication::~SevenBitEncodedCommunication() {
    delete[] _txBuffer;
    delete[] _rxBuffer;
}

SevenBitEncodedCommunication::SevenBitEncodedCommunication(SevenBitEncodedCommunication&& other) noexcept
    : _inner(other._inner), _txBuffer(other._txBuffer), _rxBuffer(other._rxBuffer), _txSize(other._txSize),
      _rxSize(other._rxSize), _rxIndex(other._rxIndex) {
    other._txBuffer = nullptr;
    other._rxBuffer = nullptr;
    other._txSize = 0;
    other._rxSize = 0;
    other._rxIndex = 0;
}

SevenBitEncodedCommunication& SevenBitEncodedCommunication::operator=(SevenBitEncodedCommunication&& other) noexcept {
    if (this != &other) {
        delete[] _txBuffer;
        delete[] _rxBuffer;

        _txBuffer = other._txBuffer;
        _rxBuffer = other._rxBuffer;
        _txSize = other._txSize;
        _rxSize = other._rxSize;
        _rxIndex = other._rxIndex;

        other._txBuffer = nullptr;
        other._rxBuffer = nullptr;
        other._txSize = 0;
        other._rxSize = 0;
        other._rxIndex = 0;
    }
    return *this;
}

bool SevenBitEncodedCommunication::writeMessage(const uint8_t* data, size_t length) {
    if ((_txBuffer == nullptr) || (_txSize == 0U)) {
        return false;
    }

    const size_t Needed = SevenBitEncoding::getEncodedBufferSize(length);
    if (Needed > _txSize) {
        return false;
    }

    const size_t EncodedLen = SevenBitEncoding::encodeBuffer(data, length, _txBuffer);

    _inner.write(_txBuffer, EncodedLen);
    return true;
}

bool SevenBitEncodedCommunication::readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen) {
    outLen = 0;
    if ((_rxBuffer == nullptr) || (_rxSize == 0U)) {
        return false;
    }

    const size_t Available = _inner.available();
    if (_rxIndex < _rxSize && Available != 0) {
        const size_t Space = _rxSize - _rxIndex;
        const size_t ToRead = std::min(Available, Space);
        if (ToRead == 0) {
            return false;
        }

        const size_t Read = _inner.read(_rxBuffer + _rxIndex, ToRead);
        if (Read == 0) {
            return false;
        }

        _rxIndex += Read;
    }

    if (_rxIndex == 0) {
        return false;
    }

    size_t encodedLen = 0;
    bool found = false;

    for (size_t i = 0; i < _rxIndex; ++i) {
        if (SevenBitEncoding::isLastByte(_rxBuffer[i])) {
            encodedLen = i + 1;
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    const size_t DecodedLen = SevenBitEncoding::decodeBuffer(_rxBuffer, encodedLen, out, maxOutLen);

    if (DecodedLen == 0 && encodedLen != 0) {
        const size_t Remaining = _rxIndex - encodedLen;
        if (Remaining > 0) {
            std::memmove(_rxBuffer, _rxBuffer + encodedLen, Remaining);
        }
        _rxIndex = Remaining;
        return false;
    }

    outLen = DecodedLen;

    const size_t Remaining = _rxIndex - encodedLen;
    if (Remaining > 0) {
        std::memmove(_rxBuffer, _rxBuffer + encodedLen, Remaining);
    }
    _rxIndex = Remaining;

    return true;
}
