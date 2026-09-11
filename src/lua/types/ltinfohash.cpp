#include "../types.hpp"

#include <optional>

#include <libtorrent/hex.hpp>
#include <libtorrent/info_hash.hpp>

using porla::Lua::Types::LtInfoHash;

namespace
{
    template <typename THash>
    THash ParseInfoHash(std::string_view hex, std::string_view label)
    {
        constexpr std::size_t chars = THash::size() * 2;

        if (hex.size() != chars)
        {
            throw std::invalid_argument(
                "info hash " + std::string(label) + " must be "
                + std::to_string(chars) + " hex characters, got "
                + std::to_string(hex.size()));
        }

        THash out;

        if (!lt::aux::from_hex({hex.data(), chars}, out.data()))
        {
            throw std::invalid_argument(
                "info hash " + std::string(label) + " contains invalid hex characters");
        }

        return out;
    }

    constexpr std::size_t kV1Chars = lt::sha1_hash::size() * 2;
    constexpr std::size_t kV2Chars = lt::sha256_hash::size() * 2;
}

void LtInfoHash::Register(sol::state& lua)
{
    lua.new_usertype<lt::info_hash_t>(
        "LtInfoHash",
        sol::call_constructor,
        sol::factories([](const std::string& hex1, sol::optional<std::string> hex2) -> lt::info_hash_t
        {
            if (hex2)
            {
                return { ParseInfoHash<lt::sha1_hash>(hex1, "v1"),
                         ParseInfoHash<lt::sha256_hash>(*hex2, "v2") };
            }
        
            switch (hex1.size())
            {
            case kV1Chars: return lt::info_hash_t(ParseInfoHash<lt::sha1_hash>(hex1, "v1"));
            case kV2Chars: return lt::info_hash_t(ParseInfoHash<lt::sha256_hash>(hex1, "v2"));
            }

            throw std::invalid_argument(
                "info hash must be " + std::to_string(kV1Chars) + " (v1) or "
                + std::to_string(kV2Chars) + " (v2) hex characters, got "
                + std::to_string(hex1.size()));
        }),
        "v1", sol::property([](const lt::info_hash_t& ih) -> std::optional<std::string>
        {
            return ih.has_v1() ? std::optional(lt::aux::to_hex(ih.v1)) : std::nullopt;
        }),
        "v2", sol::property([](const lt::info_hash_t& ih) -> std::optional<std::string>
        {
            return ih.has_v2() ? std::optional(lt::aux::to_hex(ih.v2)) : std::nullopt;
        }));
}
