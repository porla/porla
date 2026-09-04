#include "../types.hpp"

#include <optional>

#include <libtorrent/hex.hpp>
#include <libtorrent/info_hash.hpp>

using porla::Lua::Types::LtInfoHash;

void LtInfoHash::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["info_hash_t"] = lua.new_usertype<lt::info_hash_t>(
        "lt.info_hash_t",
        sol::call_constructor, sol::factories([](const std::string& hex) -> lt::info_hash_t
        {
            lt::info_hash_t ih;

            if (hex.size() == 40)
            {
                lt::aux::from_hex({hex.data(), 40}, ih.v1.data());
            }
            else if (hex.size() == 64)
            {
                lt::aux::from_hex({hex.data(), 64}, ih.v2.data());
            }
            else
            {
                throw std::invalid_argument("info_hash hex string must be 40 (v1) or 64 (v2) characters");
            }

            return ih;
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
