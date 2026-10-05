#include "string.hpp"

#include <algorithm>

using porla::Utils::String;

std::size_t String::Levenshtein(std::string_view a, std::string_view b)
{
    std::vector<size_t> row(b.size() + 1);

    for (size_t j = 0; j <= b.size(); j++)
    {
        row[j] = j;
    }

    for (size_t i = 1; i <= a.size(); i++)
    {
        size_t diag = row[0];
        row[0] = i;

        for (size_t j = 1; j <= b.size(); j++)
        {
            const size_t up = row[j];
            row[j] = std::min({ row[j] + 1, row[j - 1] + 1, diag + (a[i - 1] == b[j - 1] ? 0 : 1) });
            diag = up;
        }
    }

    return row[b.size()];
}

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

std::string String::ToLower(std::string_view input)
{
    std::string out(input);

    std::transform(
        out.begin(),
        out.end(),
        out.begin(),
        [](unsigned char c)
        {
            return std::tolower(c);
        });

    return out;
}
