#pragma once

#include <memory>

#include <sodium.h>
#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class SoHashState
    {
    public:
        static void Register(sol::state& lua);

        explicit SoHashState(crypto_generichash_state state)
            : m_state(std::move(state))
            , m_finalized(false)
        {
        }

    private:
        crypto_generichash_state m_state;
        bool                     m_finalized;
    };
}
