#include <cstring>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <sodium.h>

#include "../../src/auth/jwt.hpp"

using porla::Auth::Jwt;
using json = nlohmann::json;

// Tokens are built here with libsodium, not with the code under test, so these tests pin the
// wire format and the verification rules independently of how Jwt is implemented.
namespace
{
    const std::string Key   = "test-secret-key-0123456789abcdef";
    const auto        Now   = std::chrono::system_clock::time_point(std::chrono::seconds(1'800'000'000));
    const auto        NowS  = 1'800'000'000;
    const json        Header = {{"alg", "HS256"}, {"typ", "JWS"}};

    std::string Base64Url(std::string_view data)
    {
        std::string out(sodium_base64_ENCODED_LEN(data.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING), '\0');

        sodium_bin2base64(
            out.data(),
            out.size(),
            reinterpret_cast<const unsigned char*>(data.data()),
            data.size(),
            sodium_base64_VARIANT_URLSAFE_NO_PADDING);

        out.resize(std::strlen(out.c_str()));
        return out;
    }

    std::string Hs256(const std::string& input, const std::string& key)
    {
        unsigned char mac[crypto_auth_hmacsha256_BYTES];

        crypto_auth_hmacsha256_state state;
        crypto_auth_hmacsha256_init(&state, reinterpret_cast<const unsigned char*>(key.data()), key.size());
        crypto_auth_hmacsha256_update(&state, reinterpret_cast<const unsigned char*>(input.data()), input.size());
        crypto_auth_hmacsha256_final(&state, mac);

        return std::string(reinterpret_cast<const char*>(mac), sizeof(mac));
    }

    std::string Token(const json& header, const json& payload, const std::string& key = Key)
    {
        const auto input = Base64Url(header.dump()) + "." + Base64Url(payload.dump());
        return input + "." + Base64Url(Hs256(input, key));
    }

    json Claims()
    {
        return {
            {"exp", NowS + 86400},
            {"iat", NowS},
            {"iss", "porla"},
            {"scope", "*"},
            {"sub", "alice"}
        };
    }

    class JwtTest : public testing::Test
    {
    protected:
        void SetUp() override
        {
            ASSERT_NE(sodium_init(), -1);
        }
    };
}

TEST_F(JwtTest, Sign_ProducesTheExactTokenPorlaIssues)
{
    // Byte-for-byte: existing logins must stay valid when the implementation changes.
    EXPECT_EQ(Jwt::Sign(Key, "alice", std::chrono::days{1}, Now), Token(Header, Claims()));
}

TEST_F(JwtTest, Verify_SignedToken_ReturnsSubject)
{
    EXPECT_EQ(Jwt::Verify(Key, Jwt::Sign(Key, "alice", std::chrono::days{1}, Now), Now), "alice");
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, Claims()), Now), "alice");
}

TEST_F(JwtTest, Verify_WrongKey_IsRejected)
{
    EXPECT_EQ(Jwt::Verify("another-secret-key-0123456789abc", Token(Header, Claims()), Now), std::nullopt);
}

TEST_F(JwtTest, Verify_TamperedPayload_IsRejected)
{
    const auto token  = Token(Header, Claims());
    auto       forged = Claims();
    forged["sub"] = "mallory";

    const auto parts     = token.substr(0, token.find('.')) + "." + Base64Url(forged.dump());
    const auto signature = token.substr(token.rfind('.') + 1);

    EXPECT_EQ(Jwt::Verify(Key, parts + "." + signature, Now), std::nullopt);
}

TEST_F(JwtTest, Verify_TamperedSignature_IsRejected)
{
    auto token = Token(Header, Claims());
    token.back() = token.back() == 'A' ? 'B' : 'A';

    EXPECT_EQ(Jwt::Verify(Key, token, Now), std::nullopt);
}

TEST_F(JwtTest, Verify_AlgNone_IsRejected)
{
    const auto unsigned_token =
        Base64Url(json({{"alg", "none"}, {"typ", "JWS"}}).dump()) + "." + Base64Url(Claims().dump()) + ".";

    EXPECT_EQ(Jwt::Verify(Key, unsigned_token, Now), std::nullopt);
}

TEST_F(JwtTest, Verify_OtherAlgorithm_IsRejected)
{
    // An HS256 signature under a header claiming another algorithm.
    EXPECT_EQ(Jwt::Verify(Key, Token({{"alg", "HS512"}, {"typ", "JWS"}}, Claims()), Now), std::nullopt);
    EXPECT_EQ(Jwt::Verify(Key, Token({{"alg", "RS256"}, {"typ", "JWS"}}, Claims()), Now), std::nullopt);
}

TEST_F(JwtTest, Verify_WrongOrMissingIssuer_IsRejected)
{
    auto other = Claims();
    other["iss"] = "PORLA";
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, other), Now), std::nullopt);

    auto missing = Claims();
    missing.erase("iss");
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, missing), Now), std::nullopt);
}

TEST_F(JwtTest, Verify_Expiry_HasNoLeeway)
{
    const auto token = Token(Header, Claims());
    const auto exp   = Now + std::chrono::days{1};

    EXPECT_EQ(Jwt::Verify(Key, token, exp), "alice");
    EXPECT_EQ(Jwt::Verify(Key, token, exp + std::chrono::seconds{1}), std::nullopt);
}

TEST_F(JwtTest, Verify_IssuedInTheFuture_IsRejected)
{
    const auto token = Token(Header, Claims());

    EXPECT_EQ(Jwt::Verify(Key, token, Now - std::chrono::seconds{1}), std::nullopt);
}

TEST_F(JwtTest, Verify_NotBefore_IsHonoured)
{
    auto claims = Claims();
    claims["nbf"] = NowS + 60;
    const auto token = Token(Header, claims);

    EXPECT_EQ(Jwt::Verify(Key, token, Now), std::nullopt);
    EXPECT_EQ(Jwt::Verify(Key, token, Now + std::chrono::seconds{60}), "alice");
}

TEST_F(JwtTest, Verify_MissingOrNonStringSubject_IsRejected)
{
    auto missing = Claims();
    missing.erase("sub");
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, missing), Now), std::nullopt);

    auto number = Claims();
    number["sub"] = 42;
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, number), Now), std::nullopt);
}

TEST_F(JwtTest, Verify_MissingExpiry_IsRejected)
{
    auto claims = Claims();
    claims.erase("exp");
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, claims), Now), std::nullopt);
}

TEST_F(JwtTest, Verify_MissingIssuedAt_IsAccepted)
{
    auto claims = Claims();
    claims.erase("iat");
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, claims), Now), "alice");
}

TEST_F(JwtTest, Verify_NonIntegerTimeClaims_AreRejected)
{
    for (const auto* name : { "exp", "iat", "nbf" })
    {
        for (const json& value : { json("1800000000"), json(1800000000.5), json(nullptr) })
        {
            auto claims = Claims();
            claims[name] = value;

            EXPECT_EQ(Jwt::Verify(Key, Token(Header, claims), Now), std::nullopt) << name << " = " << value.dump();
        }
    }
}

TEST_F(JwtTest, Verify_ExtremeTimeClaims_DoNotOverflow)
{
    // Values near the int64 limits must be compared as seconds, not converted to a time_point
    // (nanoseconds), where they would overflow.
    auto far_future = Claims();
    far_future["exp"] = std::numeric_limits<std::int64_t>::max();
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, far_future), Now), "alice");

    auto too_large = Claims();
    too_large["exp"] = std::numeric_limits<std::uint64_t>::max();
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, too_large), Now), std::nullopt);

    auto unsigned_nbf = Claims();
    unsigned_nbf["nbf"] = std::numeric_limits<std::uint64_t>::max();
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, unsigned_nbf), Now), std::nullopt);

    auto far_past = Claims();
    far_past["iat"] = std::numeric_limits<std::int64_t>::min();
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, far_past), Now), "alice");

    auto expired_long_ago = Claims();
    expired_long_ago["exp"] = std::numeric_limits<std::int64_t>::min();
    EXPECT_EQ(Jwt::Verify(Key, Token(Header, expired_long_ago), Now), std::nullopt);
}

TEST_F(JwtTest, Verify_TypIsNotChecked)
{
    EXPECT_EQ(Jwt::Verify(Key, Token({{"alg", "HS256"}, {"typ", "JWT"}}, Claims()), Now), "alice");
    EXPECT_EQ(Jwt::Verify(Key, Token({{"alg", "HS256"}}, Claims()), Now), "alice");
}

TEST_F(JwtTest, Verify_Malformed_IsRejectedWithoutThrowing)
{
    const auto valid = Token(Header, Claims());

    for (const std::string token : {
             std::string(""),
             std::string("abc"),
             std::string("a.b"),
             std::string("a.b.c"),
             std::string("!!!.???.###"),
             Base64Url("not json") + "." + Base64Url("not json") + ".",
             valid + ".extra",
             valid.substr(0, valid.size() - 5)
         })
    {
        EXPECT_NO_THROW({
            EXPECT_EQ(Jwt::Verify(Key, token, Now), std::nullopt) << token;
        });
    }
}
