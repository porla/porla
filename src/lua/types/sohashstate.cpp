#include "sohashstate.hpp"

#include <sodium.h>

using porla::Lua::Types::SoHashState;

void SoHashState::Register(sol::state& lua)
{
    lua.new_usertype<SoHashState>(
        "SoHashState",
        sol::no_constructor,
        "final", [](SoHashState& state)
        {
            if (state.m_finalized)
            {
                throw sol::error("hash state already finalized");
            }

            state.m_finalized = true;

            std::string output;
            output.resize(crypto_generichash_BYTES, '\0');

            if (crypto_generichash_final(
                &state.m_state,
                reinterpret_cast<unsigned char*>(output.data()),
                output.size()) != 0)
            {
                throw sol::error("failed to finalize state");
            }

            return output;
        },
        "update", [](SoHashState& state, const std::string& data)
        {
            if (state.m_finalized)
            {
                throw sol::error("hash state already finalized");
            }

            if (crypto_generichash_update(
                &state.m_state,
                reinterpret_cast<const unsigned char*>(data.c_str()),
                data.size()) != 0)
            {
                throw sol::error("failed to update hash");
            }
        });
}
