#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

namespace porla::Workflows
{
    class ActionFactory;
    class ContextProvider;

    class IWorkflow
    {
    public:
        static std::shared_ptr<IWorkflow> LoadFromFile(const std::filesystem::path& workflow_file);

        virtual ~IWorkflow() = default;

        virtual void Execute(
            const ActionFactory& action_factory,
            const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts) = 0;

        virtual std::unordered_set<std::string> On() = 0;
    };
}
