#pragma once

#include <map>
#include <memory>
#include <string>

namespace porla::Workflows
{
    class ContextProvider;

    class TextRenderer
    {
    public:
        explicit TextRenderer(const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts);

        [[maybe_unused]] std::string Render(const std::string& text);

    private:
        std::map<std::string, std::shared_ptr<ContextProvider>> m_contexts;
    };
}
