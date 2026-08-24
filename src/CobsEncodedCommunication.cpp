#include "IntegralCommunication/CobsEncodedCommunication.h"
#include "IntegralCommunication/Encoding/CobsEncoding.h"

#include <algorithm>
#include <cstring>

CobsEncodedCommunication::CobsEncodedCommunication(Communication& inner, size_t txSize, size_t rxSize)
    : _inner(inner), _txBuffer((txSize != 0U) ? new uint8_t[txSize] : nullptr),
      _rxBuffer((rxSize != 0U) ? new uint8_t[rxSize] : nullptr), _txSize(txSize), _rxSize(rxSize), _rxIndex(0) {}

CobsEncodedCommunication::~CobsEncodedCommunication() {
    delete[] _txBuffer;
    delete[] _rxBuffer;
}

CobsEncodedCommunication::CobsEncodedCommunication(CobsEncodedCommunication&& other) noexcept
    : _inner(other._inner), _txBuffer(other._txBuffer), _rxBuffer(other._rxBuffer), _txSize(other._txSize),
      _rxSize(other._rxSize), _rxIndex(other._rxIndex) {
    other._txBuffer = nullptr;
    other._rxBuffer = nullptr;
    other._txSize = 0;
    other._rxSize = 0;
    other._rxIndex = 0;
}

CobsEncodedCommunication& CobsEncodedCommunication::operator=(CobsEncodedCommunication&& other) noexcept {
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

bool CobsEncodedCommunication::writeMessage(const uint8_t* data, size_t length) {
    if ((_txBuffer == nullptr) || (_txSize == 0U)) {
        return false;
    }

    const size_t encodedCapacity = CobsEncoding::getEncodedBufferSize(length);
    const size_t needed = encodedCapacity + 1U;
    if (needed > _txSize) {
        return false;
    }

    const size_t encodedLen = CobsEncoding::encodeBuffer(data, length, _txBuffer);
    if (encodedLen == 0U) {
        return false;
    }

    _txBuffer[encodedLen] = CobsEncoding::Delimiter;
    _inner.write(_txBuffer, encodedLen + 1U);
    return true;
}

bool CobsEncodedCommunication::readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen) {
    outLen = 0;
    if ((_rxBuffer == nullptr) || (_rxSize == 0U)) {
        return false;
    }

    const size_t available = _inner.available();
    if (_rxIndex < _rxSize && available != 0U) {
        const size_t space = _rxSize - _rxIndex;
        const size_t toRead = std::min(available, space);
        if (toRead == 0U) {
            return false;
        }

        const size_t read = _inner.read(_rxBuffer + _rxIndex, toRead);
        if (read == 0U) {
            return false;
        }

        _rxIndex += read;
    }

    if (_rxIndex == 0U) {
        return false;
    }

    size_t frameLen = 0;
    bool found = false;

    for (size_t i = 0; i < _rxIndex; i++) {
        if (CobsEncoding::isDelimiter(_rxBuffer[i])) {
            frameLen = i;
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    size_t decodedLen = 0;
    const bool decoded = CobsEncoding::decodeBuffer(_rxBuffer, frameLen, out, maxOutLen, decodedLen);

    const size_t consumed = frameLen + 1U;
    const size_t remaining = _rxIndex - consumed;
    if (remaining > 0U) {
        std::memmove(_rxBuffer, _rxBuffer + consumed, remaining);
    }
    _rxIndex = remaining;

    if (!decoded) {
        return false;
    }

    outLen = decodedLen;
    return true;
}
