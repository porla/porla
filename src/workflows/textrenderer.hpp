#pragma once

#include <map>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

namespace porla::Workflows
{
    class ContextProvider;

    class TextRenderer
    {
    public:
        explicit TextRenderer(const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts);

        [[nodiscard]] nlohmann::json Render(const std::string& text, bool raw_expression = false);

    private:
        nlohmann::json RenderSingleExpression(const std::string& expression);

        std::map<std::string, std::shared_ptr<ContextProvider>> m_contexts;
    };
}
