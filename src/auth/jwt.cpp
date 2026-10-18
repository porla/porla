#include "jwt.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include <nlohmann/json.hpp>
#include <sodium.h>

using porla::Auth::Jwt;

namespace
{
    const std::string kIssuer = "porla";

    using Mac = std::array<unsigned char, crypto_auth_hmacsha256_BYTES>;

    std::string ToBase64Url(std::string_view data)
    {
        std::string out(sodium_base64_ENCODED_LEN(data.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING), '\0');

        sodium_bin2base64(
            out.data(),
            out.size(),
            reinterpret_cast<const unsigned char*>(data.data()),
            data.size(),
            sodium_base64_VARIANT_URLSAFE_NO_PADDING);

        out.pop_back();

        return out;
    }

    std::optional<std::string> FromBase64Url(std::string_view data)
    {
        std::string out(data.size(), '\0');

        std::size_t length = 0;
        const char* end    = nullptr;

        if (sodium_base642bin(
            reinterpret_cast<unsigned char*>(out.data()),
            out.size(),
            data.data(),
            data.size(),
            nullptr,
            &length,
            &end,
            sodium_base64_VARIANT_URLSAFE_NO_PADDING) != 0
            || end  != data.data() + data.size())
        {
            return std::nullopt;
        }

        out.resize(length);

        return out;
    }

    Mac Hs256(std::string_view key, std::string_view input)
    {
        Mac mac{};

        crypto_auth_hmacsha256_state state;
        crypto_auth_hmacsha256_init(&state, reinterpret_cast<const unsigned char*>(key.data()), key.size());
        crypto_auth_hmacsha256_update(&state, reinterpret_cast<const unsigned char*>(input.data()), input.size());
        crypto_auth_hmacsha256_final(&state, mac.data());

        return mac;
    }

    std::int64_t ToSeconds(std::chrono::system_clock::time_point time)
    {
        return std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
    }

    std::optional<std::int64_t> TimeClaim(const nlohmann::json& value)
    {
        if (value.is_number_unsigned() && value.get<std::uint64_t>() > static_cast<std::uint64_t>(INT64_MAX))
        {
            return std::nullopt;
        }

        if (!value.is_number_integer())
        {
            return std::nullopt;
        }

        return value.get<std::int64_t>();
    }
}

std::string Jwt::Sign(
    const std::string&                    secret_key,
    const std::string&                    subject,
    std::chrono::seconds                  ttl,
    std::chrono::system_clock::time_point now)
{
    const nlohmann::json header = {
        {"alg", "HS256"},
        {"typ", "JWS"}
    };

    const nlohmann::json payload = {
        {"exp", ToSeconds(now + ttl)},
        {"iat", ToSeconds(now)},
        {"iss", kIssuer},
        {"sub", subject},
        {"scope", "*"}
    };

    const auto input = ToBase64Url(header.dump()) + "." + ToBase64Url(payload.dump());
    const auto mac   = Hs256(secret_key, input);

    return input + "." + ToBase64Url(std::string_view(reinterpret_cast<const char*>(mac.data()), mac.size()));
}

std::optional<std::string> Jwt::Verify(
    const std::string&                    secret_key,
    const std::string&                    token,
    std::chrono::system_clock::time_point now)
{
    const auto header_end = token.find('.');
    const auto payload_end = header_end == std::string::npos
        ? std::string::npos
        : token.find('.', header_end + 1);

    if (payload_end == std::string::npos || token.find('.', payload_end + 1) != std::string::npos)
    {
        return std::nullopt;
    }

    const auto view         = std::string_view(token);
    const auto header_json  = FromBase64Url(view.substr(0, header_end));
    const auto payload_json = FromBase64Url(view.substr(header_end + 1, payload_end - header_end - 1));
    const auto signature    = FromBase64Url(view.substr(payload_end + 1));

    if (!header_json || !payload_json || !signature || signature->size() != crypto_auth_hmacsha256_BYTES)
    {
        return std::nullopt;
    }

    const auto header = nlohmann::json::parse(*header_json, nullptr, false);

    // we only allow HS256
    if (!header.is_object() || !header.contains("alg") || header["alg"] != "HS256")
    {
        return std::nullopt;
    }

    const auto expected = Hs256(secret_key, view.substr(0, payload_end));

    if (sodium_memcmp(expected.data(), signature->data(), expected.size()) != 0)
    {
        return std::nullopt;
    }

    const auto payload = nlohmann::json::parse(*payload_json, nullptr, false);

    if (!payload.is_object()
        || !payload.contains("iss") || payload["iss"] != kIssuer
        || !payload.contains("sub") || !payload["sub"].is_string()
        || !payload.contains("exp"))
    {
        return std::nullopt;
    }

    const auto exp = TimeClaim(payload["exp"]);
    const auto iat = payload.contains("iat") ? TimeClaim(payload["iat"]) : std::optional<std::int64_t>(INT64_MIN);
    const auto nbf = payload.contains("nbf") ? TimeClaim(payload["nbf"]) : std::optional<std::int64_t>(INT64_MIN);

    if (!exp || !iat || !nbf)
    {
        return std::nullopt;
    }

    const auto now_s = ToSeconds(now);

    if (now_s > *exp || now_s < *iat || now_s < *nbf)
    {
        return std::nullopt;
    }

    return payload["sub"].get<std::string>();
}
