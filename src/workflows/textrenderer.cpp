#include "textrenderer.hpp"

#include <regex>

#include "contextprovider.hpp"

using porla::Workflows::TextRenderer;

TextRenderer::TextRenderer(const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts)
    : m_contexts(contexts)
{
}

std::string TextRenderer::Render(const std::string &text)
{
    std::regex re(R"(\$\{\{\s([a-zA-Z\.]+[^\.])\s\}\})");
    std::smatch string_match;
    std::string current = text;

    while (std::regex_search(current, string_match, re))
    {
        const std::string lookup_value = string_match[1];
        const std::string context_name = lookup_value.substr(
            0,
            lookup_value.find_first_of('.'));

        const auto resolved_value = m_contexts.at(context_name)->ResolveSegments({""});

        if (resolved_value == nullptr)
        {

        }

        break;
    }

    return current;
}
