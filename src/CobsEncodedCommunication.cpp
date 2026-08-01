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

    const size_t EncodedCapacity = CobsEncoding::getEncodedBufferSize(length);
    const size_t Needed = EncodedCapacity + 1U;
    if (Needed > _txSize) {
        return false;
    }

    const size_t EncodedLen = CobsEncoding::encodeBuffer(data, length, _txBuffer);
    if (EncodedLen == 0U) {
        return false;
    }

    _txBuffer[EncodedLen] = CobsEncoding::Delimiter;
    _inner.write(_txBuffer, EncodedLen + 1U);
    return true;
}

bool CobsEncodedCommunication::readMessage(uint8_t* out, size_t maxOutLen, size_t& outLen) {
    outLen = 0;
    if ((_rxBuffer == nullptr) || (_rxSize == 0U)) {
        return false;
    }

    const size_t Available = _inner.available();
    if (_rxIndex < _rxSize && Available != 0U) {
        const size_t Space = _rxSize - _rxIndex;
        const size_t ToRead = std::min(Available, Space);
        if (ToRead == 0U) {
            return false;
        }

        const size_t Read = _inner.read(_rxBuffer + _rxIndex, ToRead);
        if (Read == 0U) {
            return false;
        }

        _rxIndex += Read;
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
    const bool Decoded = CobsEncoding::decodeBuffer(_rxBuffer, frameLen, out, maxOutLen, decodedLen);

    const size_t Consumed = frameLen + 1U;
    const size_t Remaining = _rxIndex - Consumed;
    if (Remaining > 0U) {
        std::memmove(_rxBuffer, _rxBuffer + Consumed, Remaining);
    }
    _rxIndex = Remaining;

    if (!Decoded) {
        return false;
    }

    outLen = decodedLen;
    return true;
}
