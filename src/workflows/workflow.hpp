#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>

#include "step.hpp"

namespace porla::Workflows
{
    class ActionFactory;
    class ContextProvider;

    struct WorkflowOptions
    {
        std::string                     condition;
        std::unordered_set<std::string> on;
        std::vector<Step>               steps;
    };

    class Workflow
    {
    public:
        static std::shared_ptr<Workflow> LoadFromFile(const std::filesystem::path& workflow_file);
        static std::shared_ptr<Workflow> LoadFromYaml(const std::string& yaml);

        explicit Workflow(const WorkflowOptions& opts);
        ~Workflow();

        bool ShouldExecute(
            const std::string& event_name,
            const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts);

        void Execute(
            const ActionFactory& action_factory,
            const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts);

    private:
        std::unordered_set<std::string> m_on;
        std::string m_condition;
        std::vector<Step> m_steps;
    };
}
