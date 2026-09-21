#pragma once

#include <string>

#include <sodium.h>

namespace porla::Utils
{
        inline std::string ToHex(std::string_view in)
    {
        std::string out(in.size() * 2 + 1, '\0');

        sodium_bin2hex(
            out.data(),
            out.size(),
            reinterpret_cast<const unsigned char*>(in.data()),
            in.size());

        out.resize(in.size() * 2);   // drop the NUL
        return out;
    }

    inline bool FromHex(std::string_view in, char* out, size_t out_len)
    {
        size_t bin_len = 0;
        const char* hex_end = nullptr;

        const int res = sodium_hex2bin(
            reinterpret_cast<unsigned char*>(out),
            out_len,
            in.data(),
            in.size(),
            nullptr,
            &bin_len,
            &hex_end);

        return res == 0
            && bin_len == out_len
            && hex_end == in.data() + in.size();
    }
}
