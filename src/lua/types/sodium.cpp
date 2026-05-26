#include "../types.hpp"

#include <sodium.h>

using porla::Lua::Types::Sodium;

void Sodium::Register(sol::state& lua)
{
    sol::table sodium = lua["sodium"].valid()
        ? lua["sodium"].get<sol::table>()
        : lua.create_named_table("sodium");

    sodium["pwhash_str"] = [](const std::string& input)
    {
        std::string password_hashed;
        password_hashed.resize(crypto_pwhash_STRBYTES);

        int result = crypto_pwhash_str(
            password_hashed.data(),
            input.c_str(),
            input.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_MIN);

        if (result != 0)
        {
            throw sol::error("out of memory");
        }

        return password_hashed;
    };
}
