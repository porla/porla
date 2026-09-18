#include "sohashstate.hpp"

#include <sodium.h>

using porla::Lua::Types::SoHashState;

void SoHashState::Register(sol::state& lua)
{
    lua.new_usertype<crypto_generichash_state>(
        "SoHashState",
        sol::no_constructor,
        "final", [](crypto_generichash_state& state)
        {
            std::string output;
            output.resize(crypto_generichash_BYTES, '\0');

            crypto_generichash_final(
                &state,
                reinterpret_cast<unsigned char*>(output.data()),
                output.size());

            return output;
        },
        "update", [](crypto_generichash_state& state, const std::string& data)
        {
            crypto_generichash_update(
                &state,
                reinterpret_cast<const unsigned char*>(data.c_str()),
                data.size());
        });
}
