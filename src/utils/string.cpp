#include "string.hpp"

using porla::Utils::String;

// https://stackoverflow.com/a/46931770
std::vector<std::string> String::Split(const std::string &val, const std::string& delim)
{
    std::size_t pos_start = 0;
    std::size_t pos_end;
    std::size_t delim_len = delim.size();

    std::string token;
    std::vector<std::string> result;

    while ((pos_end = val.find(delim, pos_start)) != std::string::npos)
    {
        token = val.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        result.push_back (token);
    }

    result.push_back (val.substr (pos_start));

    return result;
}
