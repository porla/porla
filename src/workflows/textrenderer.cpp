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

std::string TextRenderer::Render(const std::string &text)
{
    std::regex re(R"(\$\{\{\s(.+?)\s\}\})");
    std::smatch string_match;
    std::string current = text;
    std::stringstream output;

    while (std::regex_search(current, string_match, re))
    {
        const auto lookup_value = string_match[1];

        output << string_match.prefix();

        jsonnet::Jsonnet jn;
        jn.init();

        std::stringstream wrapped_jsonnet;

        for (const auto& [key,val] : m_contexts)
        {
            jn.bindExtCodeVar(key, val->Value().dump());
            wrapped_jsonnet << "local " << key << " = std.extVar(\"" << key << "\");\n";
        }

        wrapped_jsonnet << lookup_value;

        std::string jsonnet_output;
        if (!jn.evaluateSnippet("workflow-snippet", wrapped_jsonnet.str(), &jsonnet_output))
        {
            printf("%s\n", jn.lastError().c_str());
        }

        const auto jsonnet_parsed_output = nlohmann::json::parse(jsonnet_output);

        if (jsonnet_parsed_output.is_string())
        {
            output << jsonnet_parsed_output.get<std::string>();
        }
        else
        {
            output << jsonnet_parsed_output;
        }

        current = string_match.suffix();
    }

    output << current;

    return output.str();
}
