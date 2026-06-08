#include "IntegralCommunication/Encoding/CobsEncoding.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <random>
#include <vector>

struct CobsBufferTestCase {
    std::vector<uint8_t> input;
    std::vector<uint8_t> expectedEncoded;
};

std::vector<uint8_t> byteRange(const uint8_t first, const uint8_t last) {
    std::vector<uint8_t> bytes;
    for (uint16_t byte = first; byte <= last; byte++) {
        bytes.push_back(static_cast<uint8_t>(byte));
    }
    return bytes;
}

std::vector<uint8_t> bytesWithPrefix(const std::vector<uint8_t>& bytes, const uint8_t prefix) {
    std::vector<uint8_t> result;
    result.reserve(bytes.size() + 1);
    result.push_back(prefix);
    result.insert(result.end(), bytes.begin(), bytes.end());
    return result;
}

std::vector<uint8_t> bytesWithSuffix(const std::vector<uint8_t>& bytes, const uint8_t suffix) {
    std::vector<uint8_t> result = bytes;
    result.push_back(suffix);
    return result;
}

std::vector<uint8_t> bytesWithPrefixAndSuffix(const std::vector<uint8_t>& bytes,
                                              const uint8_t prefix,
                                              const uint8_t suffix) {
    return bytesWithSuffix(bytesWithPrefix(bytes, prefix), suffix);
}

std::vector<uint8_t> bytesWithPrefixes(const std::vector<uint8_t>& bytes,
                                       const uint8_t firstPrefix,
                                       const uint8_t secondPrefix) {
    std::vector<uint8_t> result;
    result.reserve(bytes.size() + 2);
    result.push_back(firstPrefix);
    result.push_back(secondPrefix);
    result.insert(result.end(), bytes.begin(), bytes.end());
    return result;
}

std::vector<uint8_t> bytesWithSuffixes(const std::vector<uint8_t>& bytes,
                                       const uint8_t firstSuffix,
                                       const uint8_t secondSuffix) {
    std::vector<uint8_t> result = bytes;
    result.push_back(firstSuffix);
    result.push_back(secondSuffix);
    return result;
}

std::vector<uint8_t> bytesWithPrefixAndSuffixes(const std::vector<uint8_t>& bytes,
                                                const uint8_t prefix,
                                                const uint8_t firstSuffix,
                                                const uint8_t secondSuffix) {
    std::vector<uint8_t> result = bytesWithPrefixAndSuffix(bytes, prefix, firstSuffix);
    result.push_back(secondSuffix);
    return result;
}

class CobsBufferEncodingTest : public ::testing::TestWithParam<CobsBufferTestCase> {};

TEST_P(CobsBufferEncodingTest, EncodeBuffer) {
    const auto& testCase = GetParam();
    std::vector<uint8_t> encoded(CobsEncoding::getEncodedBufferSize(testCase.input.size()));

    const size_t encodedLength = CobsEncoding::encodeBuffer(testCase.input.data(), testCase.input.size(), encoded.data());

    encoded.resize(encodedLength);
    EXPECT_EQ(encoded, testCase.expectedEncoded);
}

TEST_P(CobsBufferEncodingTest, DecodeBuffer) {
    const auto& testCase = GetParam();
    std::vector<uint8_t> decoded(testCase.input.size());
    size_t decodedLength = 0;

    const bool ok = CobsEncoding::decodeBuffer(testCase.expectedEncoded.data(),
                                               testCase.expectedEncoded.size(),
                                               decoded.data(),
                                               decoded.size(),
                                               decodedLength);

    decoded.resize(decodedLength);
    EXPECT_TRUE(ok);
    EXPECT_EQ(decoded, testCase.input);
}

INSTANTIATE_TEST_SUITE_P(
    CobsEncoding,
    CobsBufferEncodingTest,
    ::testing::Values(
        CobsBufferTestCase{{}, {0x01}},
        CobsBufferTestCase{{0x00}, {0x01, 0x01}},
        CobsBufferTestCase{{0x00, 0x00}, {0x01, 0x01, 0x01}},
        CobsBufferTestCase{{0x00, 0x11, 0x00}, {0x01, 0x02, 0x11, 0x01}},
        CobsBufferTestCase{{0x11, 0x22, 0x00, 0x33}, {0x03, 0x11, 0x22, 0x02, 0x33}},
        CobsBufferTestCase{{0x11, 0x22, 0x33, 0x44}, {0x05, 0x11, 0x22, 0x33, 0x44}},
        CobsBufferTestCase{{0x11, 0x00, 0x00, 0x00}, {0x02, 0x11, 0x01, 0x01, 0x01}},
        CobsBufferTestCase{byteRange(0x01, 0xFE), bytesWithPrefix(byteRange(0x01, 0xFE), 0xFF)},
        CobsBufferTestCase{bytesWithPrefix(byteRange(0x01, 0xFE), 0x00),
                           bytesWithPrefixes(byteRange(0x01, 0xFE), 0x01, 0xFF)},
        CobsBufferTestCase{byteRange(0x01, 0xFF),
                           bytesWithPrefixAndSuffixes(byteRange(0x01, 0xFE), 0xFF, 0x02, 0xFF)},
        CobsBufferTestCase{bytesWithSuffix(byteRange(0x02, 0xFF), 0x00),
                           bytesWithPrefixAndSuffixes(byteRange(0x02, 0xFF), 0xFF, 0x01, 0x01)},
        CobsBufferTestCase{bytesWithSuffixes(byteRange(0x03, 0xFF), 0x00, 0x01),
                           bytesWithPrefixAndSuffixes(byteRange(0x03, 0xFF), 0xFE, 0x02, 0x01)}));

TEST(CobsEncoding, EncodesLongNonZeroBlock) {
    std::vector<uint8_t> input(CobsEncoding::MaxBlockPayloadSize, 0x7A);
    std::vector<uint8_t> encoded(CobsEncoding::getEncodedBufferSize(input.size()));

    const size_t encodedLength = CobsEncoding::encodeBuffer(input.data(), input.size(), encoded.data());

    ASSERT_EQ(encodedLength, input.size() + 1);
    EXPECT_EQ(encoded[0], 0xFF);
    EXPECT_EQ(encoded[encodedLength - 1], 0x7A);
}

TEST(CobsEncoding, EncodesMoreThanOneFullNonZeroBlock) {
    std::vector<uint8_t> input(300, 0x7A);
    std::vector<uint8_t> encoded(CobsEncoding::getEncodedBufferSize(input.size()));

    const size_t encodedLength = CobsEncoding::encodeBuffer(input.data(), input.size(), encoded.data());

    ASSERT_EQ(encodedLength, input.size() + 2);
    EXPECT_EQ(encoded[0], CobsEncoding::FullBlockCode);
    EXPECT_EQ(encoded[CobsEncoding::FullBlockCode], 47);

    for (size_t i = 1; i < CobsEncoding::FullBlockCode; i++) {
        EXPECT_EQ(encoded[i], 0x7A);
    }
    for (size_t i = CobsEncoding::FullBlockCode + 1; i < encodedLength; i++) {
        EXPECT_EQ(encoded[i], 0x7A);
    }
}

TEST(CobsEncoding, DecodesMoreThanOneFullNonZeroBlock) {
    std::vector<uint8_t> encoded(302, 0x7A);
    encoded[0] = CobsEncoding::FullBlockCode;
    encoded[CobsEncoding::FullBlockCode] = 47;
    std::vector<uint8_t> decoded(300);
    size_t decodedLength = 0;

    const bool ok = CobsEncoding::decodeBuffer(encoded.data(), encoded.size(), decoded.data(), decoded.size(), decodedLength);

    EXPECT_TRUE(ok);
    EXPECT_EQ(decodedLength, decoded.size());
    for (uint8_t byte : decoded) {
        EXPECT_EQ(byte, 0x7A);
    }
}

TEST(CobsEncoding, EncodesMoreThan256BytesWithSecondCodeAfterZero) {
    std::vector<uint8_t> input(300, 0x7A);
    input[260] = CobsEncoding::Delimiter;
    std::vector<uint8_t> encoded(CobsEncoding::getEncodedBufferSize(input.size()));

    const size_t encodedLength = CobsEncoding::encodeBuffer(input.data(), input.size(), encoded.data());

    ASSERT_EQ(encodedLength, input.size() + 2);
    EXPECT_EQ(encoded[0], CobsEncoding::FullBlockCode);
    EXPECT_EQ(encoded[CobsEncoding::FullBlockCode], 7);
    EXPECT_EQ(encoded[262], 40);
}

TEST(CobsEncoding, DecodeBufferRejectsMalformedPayloads) {
    size_t decodedLength = 0;
    uint8_t output[4] = {};

    const uint8_t containsDelimiter[] = {0x02, 0x00};
    EXPECT_FALSE(CobsEncoding::decodeBuffer(containsDelimiter, sizeof(containsDelimiter), output, sizeof(output), decodedLength));

    const uint8_t blockRunsPastInput[] = {0x03, 0x11};
    EXPECT_FALSE(CobsEncoding::decodeBuffer(blockRunsPastInput, sizeof(blockRunsPastInput), output, sizeof(output), decodedLength));
}

TEST(CobsEncoding, DecodeBufferRejectsSmallOutputBuffer) {
    const uint8_t input[] = {0x03, 0x11, 0x22};
    uint8_t output[1] = {};
    size_t decodedLength = 0;

    EXPECT_FALSE(CobsEncoding::decodeBuffer(input, sizeof(input), output, sizeof(output), decodedLength));
}

TEST(CobsEncoding, FuzzEncodeDecode) {
    std::mt19937 rng(0xC0B5);
    std::uniform_int_distribution<size_t> sizeDist(0, 512);
    std::uniform_int_distribution<int> byteDist(0, 255);

    for (int i = 0; i < 1000; i++) {
        std::vector<uint8_t> input(sizeDist(rng));
        for (uint8_t& byte : input) {
            byte = static_cast<uint8_t>(byteDist(rng));
        }

        std::vector<uint8_t> encoded(CobsEncoding::getEncodedBufferSize(input.size()));
        const size_t encodedLength = CobsEncoding::encodeBuffer(input.data(), input.size(), encoded.data());
        encoded.resize(encodedLength);

        std::vector<uint8_t> decoded(input.size());
        size_t decodedLength = 0;
        ASSERT_TRUE(CobsEncoding::decodeBuffer(encoded.data(), encoded.size(), decoded.data(), decoded.size(), decodedLength));
        decoded.resize(decodedLength);
        EXPECT_EQ(decoded, input);
    }
}

TEST(CobsEncoding, IsDelimiter) {
    EXPECT_TRUE(CobsEncoding::isDelimiter(0x00));
    EXPECT_FALSE(CobsEncoding::isDelimiter(0x01));
}
