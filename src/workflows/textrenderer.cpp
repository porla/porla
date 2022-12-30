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
        current = current.replace(
            string_match.position(),
            string_match.size(),
            "whatever man");
    }

    return current;
}
