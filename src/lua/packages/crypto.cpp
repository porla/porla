#include "crypto.hpp"

#include <sodium.h>

using porla::Lua::Packages::Crypto;

sol::object Crypto::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("random_bytes", [](int n)
    {
        std::string bytes;
        bytes.resize(n, '\0');

        randombytes_buf(bytes.data(), n);

        return bytes;
    });

    return tbl;
}
