#include <gtest/gtest.h>

#include <ctime>

#include <sqlite3.h>

#include "../../src/auth/tokens.hpp"
#include "../../src/data/migrate.hpp"

namespace Tokens = porla::Auth::Tokens;

using Token = porla::Data::Models::Tokens::Token;

namespace
{
    class TokensTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            ASSERT_EQ(sqlite3_open(":memory:", &db), SQLITE_OK);
            ASSERT_TRUE(porla::Data::Migrate(db));
            ASSERT_EQ(sqlite3_exec(db, "INSERT INTO users (id, username, password) VALUES (1, 'u', 'x')", nullptr, nullptr, nullptr), SQLITE_OK);
        }

        void TearDown() override { sqlite3_close(db); }

        void Exec(const std::string& sql) { ASSERT_EQ(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr), SQLITE_OK); }

        std::int64_t Now() const { return std::time(nullptr); }

        Tokens::Issued Session(std::int64_t expires_at)
        {
            return Tokens::Issue(db, Token{ .kind = "session", .user_id = 1, .expires_at = expires_at, .last_used_at = Now() });
        }

        sqlite3* db = nullptr;
    };
}

TEST_F(TokensTest, IssuedTokenVerifies)
{
    const auto issued = Tokens::Issue(db, Token{ .kind = "api_key", .name = "k" });

    EXPECT_EQ(issued.token.size(), 6 + 16 + 1 + 43);
    EXPECT_TRUE(issued.token.starts_with("porla_" + issued.id + "_"));

    const auto row = Tokens::Verify(db, issued.token);
    ASSERT_TRUE(row.has_value());
    EXPECT_EQ(row->id, issued.id);
    EXPECT_EQ(row->kind, "api_key");
    EXPECT_EQ(row->name, "k");
}

TEST_F(TokensTest, SessionCarriesUsername)
{
    const auto row = Tokens::Verify(db, Session(Now() + 60).token);
    ASSERT_TRUE(row.has_value());
    EXPECT_EQ(row->kind, "session");
    EXPECT_EQ(row->username, "u");
}

TEST_F(TokensTest, RejectsWrongSecretUnknownIdAndMalformed)
{
    const auto issued = Tokens::Issue(db, Token{ .kind = "api_key", .name = "k" });

    auto wrong_secret = issued.token;
    wrong_secret.back() = wrong_secret.back() == 'A' ? 'B' : 'A';
    EXPECT_FALSE(Tokens::Verify(db, wrong_secret).has_value());

    auto unknown_id = issued.token;
    unknown_id[6] = unknown_id[6] == '0' ? '1' : '0';
    EXPECT_FALSE(Tokens::Verify(db, unknown_id).has_value());

    EXPECT_FALSE(Tokens::Verify(db, "").has_value());
    EXPECT_FALSE(Tokens::Verify(db, "eyJhbGciOiJIUzI1NiJ9.e30.x").has_value());
    EXPECT_FALSE(Tokens::Verify(db, issued.token.substr(0, issued.token.size() - 1)).has_value());
    EXPECT_FALSE(Tokens::Verify(db, issued.token + "A").has_value());
    EXPECT_FALSE(Tokens::Verify(db, "porla_" + issued.id + "_" + std::string(43, '!')).has_value());
}

TEST_F(TokensTest, ExpiredTokensAreRejected)
{
    EXPECT_FALSE(Tokens::Verify(db, Session(Now() - 1).token).has_value());

    const auto key = Tokens::Issue(db, Token{ .kind = "api_key", .name = "k", .expires_at = Now() - 1 });
    EXPECT_FALSE(Tokens::Verify(db, key.token).has_value());
}

TEST_F(TokensTest, UseSlidesSessionExpiryAtMostEveryTouchInterval)
{
    const auto issued = Session(Now() + 60);

    // used within the touch interval: nothing written
    auto row = Tokens::Verify(db, issued.token);
    ASSERT_TRUE(row.has_value());
    EXPECT_EQ(row->expires_at, porla::Data::Models::Tokens::GetById(db, issued.id)->expires_at);
    EXPECT_LE(*row->expires_at, Now() + 60);

    // last used long ago: touched, and the session slides to now + idle timeout
    Exec("UPDATE tokens SET last_used_at = " + std::to_string(Now() - 3600) + " WHERE id = '" + issued.id + "'");
    row = Tokens::Verify(db, issued.token);
    ASSERT_TRUE(row.has_value());

    const auto stored = porla::Data::Models::Tokens::GetById(db, issued.id);
    EXPECT_GE(*stored->last_used_at, Now() - 1);
    EXPECT_GE(*stored->expires_at, Now() + Tokens::kSessionIdleTimeout.count() - 1);
}

TEST_F(TokensTest, ApiKeyExpiryDoesNotSlide)
{
    const auto expires = Now() + 60;
    const auto key     = Tokens::Issue(db, Token{ .kind = "api_key", .name = "k", .expires_at = expires });

    ASSERT_TRUE(Tokens::Verify(db, key.token).has_value());

    const auto stored = porla::Data::Models::Tokens::GetById(db, key.id);
    EXPECT_EQ(stored->expires_at, expires);
    EXPECT_TRUE(stored->last_used_at.has_value());
}

TEST_F(TokensTest, RemoveExpiredSessionsKeepsApiKeys)
{
    Session(Now() - 10);
    const auto live = Session(Now() + 60);
    const auto key  = Tokens::Issue(db, Token{ .kind = "api_key", .name = "old", .expires_at = Now() - 10 });

    EXPECT_EQ(porla::Data::Models::Tokens::RemoveExpiredSessions(db, Now()), 1);
    EXPECT_TRUE(porla::Data::Models::Tokens::GetById(db, live.id).has_value());
    EXPECT_TRUE(porla::Data::Models::Tokens::GetById(db, key.id).has_value());
}
