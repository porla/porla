#include "textrenderer.hpp"

#include <regex>
#include <sstream>

#include <duktape.h>

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
    duk_context* ctx = duk_create_heap_default();
    duk_push_global_object(ctx);

    for (const auto& [key, value] : m_contexts)
    {
        // Push and decode JSON.
        duk_push_string(ctx, value->Value().dump().c_str());
        duk_json_decode(ctx, -1);

        // Set property on global object
        duk_put_prop_string(ctx, -2, key.c_str());
    }

    // Pop global object
    duk_pop(ctx);

    // Evaluate expression
    duk_peval_string(ctx, expression.c_str());

    // TODO: check result etc. Can't encode functions

    // Encode as JSON
    duk_json_encode(ctx, -1);

    // Get result
    const char* encoded_output = duk_get_string(ctx, -1);
    nlohmann::json output = nlohmann::json::parse(encoded_output);

    duk_destroy_heap(ctx);

    return output;
}
