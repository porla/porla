#include "textrenderer.hpp"

#include <regex>
#include <sstream>

#include <libjsonnet++.h>

#include "contextprovider.hpp"
#include "../utils/string.hpp"

using porla::Utils::String;
using porla::Workflows::TextRenderer;

TextRenderer::TextRenderer(const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts)
    : m_contexts(contexts)
{
}

nlohmann::json TextRenderer::Render(const std::string& text, bool raw_expression)
{
    if (raw_expression)
    {
        return RenderSingleExpression(text);
    }

    std::regex re(R"(\$\{\{\s(.+?)\s\}\})");
    std::smatch string_match;
    std::string current = text;
    std::vector<nlohmann::json> fragments;

    while (std::regex_search(current, string_match, re))
    {
        const auto lookup_value = string_match[1];

        if (string_match.prefix().length() > 0)
        {
            fragments.emplace_back(string_match.prefix());
        }

        fragments.emplace_back(RenderSingleExpression(lookup_value));

        current = string_match.suffix();
    }

    if (!current.empty())
    {
        fragments.emplace_back(current);
    }

    if (fragments.size() == 1)
    {
        return fragments.at(0);
    }
    else if (fragments.size() > 1)
    {
        std::stringstream combined_output;

        for (const auto& fragment : fragments)
        {
            if (fragment.is_string())
            {
                combined_output << fragment.get<std::string>();
            }
            else
            {
                combined_output << fragment;
            }
        }

        return combined_output.str();
    }

    return {};
}

nlohmann::json TextRenderer::RenderSingleExpression(const std::string &expression)
{
    jsonnet::Jsonnet jn;
    jn.init();

    std::stringstream wrapped_jsonnet;

    for (const auto& [key,val] : m_contexts)
    {
        jn.bindExtCodeVar(key, val->Value().dump());
        wrapped_jsonnet << "local " << key << " = std.extVar(\"" << key << "\");\n";
    }

    wrapped_jsonnet << expression;

    std::string jsonnet_output;
    if (!jn.evaluateSnippet("workflow-snippet", wrapped_jsonnet.str(), &jsonnet_output))
    {
        printf("%s\n", jn.lastError().c_str());
        return nullptr;
    }

    return nlohmann::json::parse(jsonnet_output);
}
