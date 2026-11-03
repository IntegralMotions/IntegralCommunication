#include "IntegralCommunication/BufferedCommunication.h"
#include "IntegralCommunication/CobsEncodedCommunication.h"
#include "IntegralCommunication/SevenBitEncodedCommunication.h"

#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace IntegralMotions::Diagnostics;

namespace {
class TestLogger : public Logger {
  public:
    std::vector<std::string> messages;
    void log(LogLevel level, std::string_view format, std::span<const LogValue>) override {
        EXPECT_EQ(level, LogLevel::Warning);
        messages.emplace_back(format);
    }
};

class Transport : public Communication {
  public:
    std::vector<uint8_t> incoming;
  private:
    size_t writeImpl(const uint8_t*, size_t size) override { return size; }
    size_t availableImpl() override { return incoming.size(); }
    size_t readImpl(uint8_t* data, size_t size) override {
        const size_t count = std::min(size, incoming.size());
        std::memcpy(data, incoming.data(), count);
        incoming.erase(incoming.begin(), incoming.begin() + count);
        return count;
    }
};

class BlockedTransport : public BufferedCommunication {
  public:
    using BufferedCommunication::BufferedCommunication;
  private:
    size_t writeImpl(const uint8_t*, size_t) override { return 0; }
    size_t availableImpl() override { return 0; }
    size_t readImpl(uint8_t*, size_t) override { return 0; }
};

template <typename Wrapper>
void checkDiagnostics(const char* prefix, uint8_t unterminated) {
    TestLogger logger;
    Transport transport;
    Wrapper comm(transport, 2, 2, logger);
    uint8_t out[8]{};
    size_t length = 0;
    EXPECT_FALSE(comm.readMessage(out, sizeof(out), length));
    EXPECT_TRUE(logger.messages.empty());
    const uint8_t payload[] = {1, 2, 3};
    EXPECT_FALSE(comm.writeMessage(payload, sizeof(payload)));
    transport.incoming = {unterminated, unterminated};
    EXPECT_FALSE(comm.readMessage(out, sizeof(out), length));
    EXPECT_FALSE(comm.readMessage(out, sizeof(out), length));
    ASSERT_EQ(logger.messages.size(), 2u);
    EXPECT_NE(logger.messages[0].find(std::string(prefix) + " transmit buffer too small"), std::string::npos);
    EXPECT_NE(logger.messages[1].find(std::string(prefix) + " receive buffer full"), std::string::npos);
}

TEST(CommunicationLoggingTests, CobsWarningsAndQuietPolling) {
    checkDiagnostics<CobsEncodedCommunication>("COBS", 1);
}

TEST(CommunicationLoggingTests, SevenBitWarningsAndQuietPolling) {
    checkDiagnostics<SevenBitEncodedCommunication>("7-bit", 0x80);
}

TEST(CommunicationLoggingTests, BlockedBufferedWriteWarns) {
    TestLogger logger;
    uint8_t buffer[1]{};
    BlockedTransport comm(buffer, sizeof(buffer), logger);
    const uint8_t data[] = {1, 2};
    comm.write(data, sizeof(data));
    ASSERT_EQ(logger.messages.size(), 1u);
    EXPECT_NE(logger.messages[0].find("Buffered write could not accept"), std::string::npos);
}
} // namespace
