#include "textrenderer.hpp"

#include <regex>
#include <sstream>

#include "contextprovider.hpp"
#include "../utils/string.hpp"

using porla::Utils::String;
using porla::Workflows::TextRenderer;

TextRenderer::TextRenderer(const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts)
    : m_contexts(contexts)
{
}

std::string TextRenderer::Render(const std::string &text)
{
    std::regex re(R"(\$\{\{\s([a-zA-Z0-9_\.]+[^\.])\s\}\})");
    std::smatch string_match;
    std::string current = text;
    std::stringstream output;

    while (std::regex_search(current, string_match, re))
    {
        const std::string lookup_value = string_match[1];

        const std::string context_name = lookup_value.substr(
            0,
            lookup_value.find_first_of('.'));

        const std::string raw_segments = lookup_value.substr(
            lookup_value.find_first_of('.') + 1);

        const auto segments = String::Split(raw_segments, ".");
        const auto resolved_value = m_contexts.at(context_name)->ResolveSegments(segments);

        output << string_match.prefix();

        if (resolved_value.is_string())
        {
            output << resolved_value.get<std::string>();
        }
        else
        {
            output << resolved_value;
        }

        current = string_match.suffix();
    }

    output << current;

    return output.str();
}
