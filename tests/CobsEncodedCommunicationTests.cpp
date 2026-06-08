#include "IntegralCommunication/CobsEncodedCommunication.h"
#include "IntegralCommunication/Communication.h"
#include "IntegralCommunication/Encoding/CobsEncoding.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

class CobsFakeCommunication : public Communication {
  public:
    const std::vector<uint8_t>& written() const {
        return _written;
    }

    void pushIncoming(const std::vector<uint8_t>& data) {
        _incoming.insert(_incoming.end(), data.begin(), data.end());
    }

  private:
    size_t writeImpl(const uint8_t* data, size_t size) override {
        _written.insert(_written.end(), data, data + size);
        return size;
    }

    size_t availableImpl() override {
        return _incoming.size();
    }

    size_t readImpl(uint8_t* data, size_t size) override {
        const size_t toRead = std::min(size, _incoming.size());
        if (toRead == 0U) {
            return 0;
        }

        std::memcpy(data, _incoming.data(), toRead);
        _incoming.erase(_incoming.begin(), _incoming.begin() + static_cast<std::ptrdiff_t>(toRead));
        return toRead;
    }

    std::vector<uint8_t> _written;
    std::vector<uint8_t> _incoming;
};

std::vector<uint8_t> encodeCobsFrame(const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> frame(CobsEncoding::getEncodedBufferSize(payload.size()) + 1U);
    const size_t encodedLen = CobsEncoding::encodeBuffer(payload.data(), payload.size(), frame.data());
    frame.resize(encodedLen + 1U);
    frame[encodedLen] = CobsEncoding::Delimiter;
    return frame;
}

TEST(CobsEncodedCommunicationTests, WriteMessageEncodesAndForwardsDelimitedFrame) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> payload = {0x01, 0x00, 0x02, 0xFF};
    const std::vector<uint8_t> expected = encodeCobsFrame(payload);

    ASSERT_TRUE(comm.writeMessage(payload.data(), payload.size()));
    EXPECT_EQ(fake.written(), expected);
}

TEST(CobsEncodedCommunicationTests, WriteMessageReturnsFalseWhenTransmitBufferIsTooSmall) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 4, 128);

    const std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0x04};

    EXPECT_FALSE(comm.writeMessage(payload.data(), payload.size()));
    EXPECT_TRUE(fake.written().empty());
}

TEST(CobsEncodedCommunicationTests, ReadMessageReturnsFalseWhenNoData) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 128, 128);

    uint8_t out[32] = {};
    size_t outLen = 0;

    const bool result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_FALSE(result);
    EXPECT_EQ(outLen, 0U);
}

TEST(CobsEncodedCommunicationTests, ReadMessageNonBlockingPartialThenFull) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> payload = {0x10, 0x00, 0x20, 0x30};
    const std::vector<uint8_t> encoded = encodeCobsFrame(payload);

    ASSERT_GT(encoded.size(), 1U);

    const size_t partialLen = encoded.size() - 1U;
    fake.pushIncoming(std::vector<uint8_t>(encoded.begin(), encoded.begin() + static_cast<std::ptrdiff_t>(partialLen)));

    uint8_t out[32] = {};
    size_t outLen = 0;
    bool result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_FALSE(result);
    EXPECT_EQ(outLen, 0U);

    fake.pushIncoming(std::vector<uint8_t>(encoded.begin() + static_cast<std::ptrdiff_t>(partialLen), encoded.end()));

    result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_TRUE(result);
    EXPECT_EQ(outLen, payload.size());
    for (size_t i = 0; i < payload.size(); i++) {
        EXPECT_EQ(out[i], payload[i]) << "byte " << i << " mismatch";
    }
}

TEST(CobsEncodedCommunicationTests, ReadTwoMessagesBackToBack) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> msg1 = {0x01, 0x00, 0x03};
    const std::vector<uint8_t> msg2 = {0xAA, 0xBB, 0x00, 0xDD};
    const std::vector<uint8_t> enc1 = encodeCobsFrame(msg1);
    const std::vector<uint8_t> enc2 = encodeCobsFrame(msg2);

    std::vector<uint8_t> combined;
    combined.reserve(enc1.size() + enc2.size());
    combined.insert(combined.end(), enc1.begin(), enc1.end());
    combined.insert(combined.end(), enc2.begin(), enc2.end());
    fake.pushIncoming(combined);

    uint8_t out[32] = {};
    size_t outLen = 0;

    bool result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_TRUE(result);
    EXPECT_EQ(outLen, msg1.size());
    for (size_t i = 0; i < msg1.size(); i++) {
        EXPECT_EQ(out[i], msg1[i]) << "msg1 byte " << i << " mismatch";
    }

    std::memset(out, 0, sizeof(out));
    result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_TRUE(result);
    EXPECT_EQ(outLen, msg2.size());
    for (size_t i = 0; i < msg2.size(); i++) {
        EXPECT_EQ(out[i], msg2[i]) << "msg2 byte " << i << " mismatch";
    }

    result = comm.readMessage(out, sizeof(out), outLen);
    EXPECT_FALSE(result);
    EXPECT_EQ(outLen, 0U);
}

TEST(CobsEncodedCommunicationTests, ReadsEmptyPayload) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 128, 128);

    fake.pushIncoming(encodeCobsFrame({}));

    uint8_t out[1] = {};
    size_t outLen = 123;

    EXPECT_TRUE(comm.readMessage(out, sizeof(out), outLen));
    EXPECT_EQ(outLen, 0U);
}

TEST(CobsEncodedCommunicationTests, ConsumesMalformedFrameAndContinuesWithNextFrame) {
    CobsFakeCommunication fake;
    CobsEncodedCommunication comm(fake, 128, 128);

    const std::vector<uint8_t> validPayload = {0x44, 0x00, 0x55};
    std::vector<uint8_t> incoming = {0x03, 0x11, CobsEncoding::Delimiter};
    const std::vector<uint8_t> validFrame = encodeCobsFrame(validPayload);
    incoming.insert(incoming.end(), validFrame.begin(), validFrame.end());
    fake.pushIncoming(incoming);

    uint8_t out[32] = {};
    size_t outLen = 0;

    EXPECT_FALSE(comm.readMessage(out, sizeof(out), outLen));
    EXPECT_EQ(outLen, 0U);

    EXPECT_TRUE(comm.readMessage(out, sizeof(out), outLen));
    ASSERT_EQ(outLen, validPayload.size());
    for (size_t i = 0; i < validPayload.size(); i++) {
        EXPECT_EQ(out[i], validPayload[i]) << "byte " << i << " mismatch";
    }
}
