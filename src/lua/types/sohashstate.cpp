#include "sohashstate.hpp"

#include <sodium.h>

#include "poerror.hpp"

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
                throw Error(Errors::Code::invalid_state, "hash state already finalized");
            }

            state.m_finalized = true;

            std::string output;
            output.resize(crypto_generichash_BYTES, '\0');

            if (crypto_generichash_final(
                &state.m_state,
                reinterpret_cast<unsigned char*>(output.data()),
                output.size()) != 0)
            {
                throw Error(Errors::Code::internal_error, "failed to finalize state");
            }

            return output;
        },
        "update", [](SoHashState& state, const std::string& data)
        {
            if (state.m_finalized)
            {
                throw Error(Errors::Code::invalid_state, "hash state already finalized");
            }

            if (crypto_generichash_update(
                &state.m_state,
                reinterpret_cast<const unsigned char*>(data.c_str()),
                data.size()) != 0)
            {
                throw Error(Errors::Code::internal_error, "failed to update hash");
            }
        });
}
