#include "workflow.hpp"

#include <fstream>
#include <regex>
#include <utility>
#include <vector>

#include <boost/log/trivial.hpp>
#include <ryml/ryml.hpp>

#include "action.hpp"
#include "actionfactory.hpp"
#include "contextprovider.hpp"
#include "step.hpp"

using porla::Workflows::Action;
using porla::Workflows::ActionCallback;
using porla::Workflows::ActionFactory;
using porla::Workflows::ContextProvider;
using porla::Workflows::IWorkflow;
using porla::Workflows::Step;

struct StepInstance
{
    std::shared_ptr<Action> action;
    Step                    step;
    nlohmann::json          output;
};

class SimpleActionParams : public porla::Workflows::ActionParams
{
public:
    explicit SimpleActionParams(nlohmann::json input)
        : m_input(std::move(input))
    {
    }

    [[nodiscard]] nlohmann::json Input() const override
    {
        return m_input;
    }

    [[nodiscard]] std::string RenderValues(const std::string& text) const override
    {
        std::regex re(R"(\$\{\{\s([a-zA-Z\.]+[^\.])\s\}\})");
        std::smatch string_match;
        std::string current = text;

        while (std::regex_search(current, string_match, re))
        {
            BOOST_LOG_TRIVIAL(info) << string_match[1];

            current = current.replace(
                string_match.position(),
                string_match.size(),
                "whatever man");
        }

        return text;
    }

private:
    nlohmann::json m_input;
};

class LoopingWorkflowRunner : public ActionCallback, public std::enable_shared_from_this<LoopingWorkflowRunner>
{
public:
    explicit LoopingWorkflowRunner(
        const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts,
        const std::vector<StepInstance>& step_instances)
        : m_contexts(contexts)
        , m_step_instances(step_instances)
        , m_current_index(0)
    {
    }

    void Complete(const nlohmann::json& j) override
    {
        // This is the instance that completed. Set its output.
        auto& instance = m_step_instances.at(m_current_index);
        instance.output = j;

        m_current_index++;

        if (m_current_index >= m_step_instances.size())
        {
            return;
        }

        Run();
    }

    void Run()
    {
        const auto& instance = m_step_instances.at(m_current_index);

        SimpleActionParams sap{instance.step.with};

        instance.action->Invoke(sap, shared_from_this());
    }

private:
    std::map<std::string, std::shared_ptr<ContextProvider>> m_contexts;
    std::vector<StepInstance> m_step_instances;
    int m_current_index;
};

struct YamlWorkflowOptions
{
    std::unordered_set<std::string>     on;
    std::vector<porla::Workflows::Step> steps;
};

class YamlWorkflow : public IWorkflow
{
public:
    explicit YamlWorkflow(const YamlWorkflowOptions& options)
        : m_on(options.on)
        , m_steps(options.steps)
    {
    }

    void Execute(
        const ActionFactory& action_factory,
        const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts) override
    {
        std::vector<StepInstance> step_instances;

        for (const auto& step : m_steps)
        {
            auto action = action_factory.Construct(step.uses);

            if (action == nullptr)
            {
                BOOST_LOG_TRIVIAL(error) << "Invalid action name: " << step.uses;
                return;
            }

            step_instances.emplace_back(StepInstance{
                .action = action,
                .step   = step
            });
        }

        std::make_shared<LoopingWorkflowRunner>(contexts, step_instances)->Run();
    }

    std::unordered_set<std::string> On() override
    {
        return m_on;
    }

private:
    std::unordered_set<std::string> m_on;
    std::vector<porla::Workflows::Step> m_steps;
};

std::shared_ptr<IWorkflow> IWorkflow::LoadFromFile(const std::filesystem::path& workflow_file)
{
    std::ifstream workflow_input_file(workflow_file, std::ios::binary);

    // Get the params file size
    workflow_input_file.seekg(0, std::ios_base::end);
    const std::streamsize workflow_file_size = workflow_input_file.tellg();
    workflow_input_file.seekg(0, std::ios_base::beg);

    BOOST_LOG_TRIVIAL(info) << "Reading workflow file (" << workflow_file_size << " bytes)";

    std::vector<char> workflow_file_buffer;
    workflow_file_buffer.resize(workflow_file_size);

    workflow_input_file.read(workflow_file_buffer.data(), workflow_file_size);

    ryml::Tree wf = ryml::parse_in_arena(ryml::to_csubstr(workflow_file_buffer.data()));

    auto on_val = wf["on"].val();
    std::string on(on_val.data(), on_val.size());

    const auto steps = wf["steps"];

    std::vector<Step> workflow_steps;

    if (steps.is_seq())
    {
        for (const auto step : steps)
        {
            const auto uses_val = step["uses"].val();

            std::map<std::string, nlohmann::json> with;

            for (const ryml::NodeRef& with_item : step["with"].children())
            {
                const auto key = with_item.key();
                const auto val = with_item.val();

                std::string key_str(key.data(), key.size());
                std::string val_str(val.data(), val.size());

                if (val.is_integer())
                {
                    with.insert({ key_str, std::stoi(val_str) });
                }
                else
                {
                    with.insert({ key_str, val_str });
                }
            }

            workflow_steps.emplace_back(Step{
                .uses = std::string(uses_val.data(), uses_val.size()),
                .with = with
            });
        }
    }

    return std::make_shared<YamlWorkflow>(YamlWorkflowOptions{
        .on    = {on},
        .steps = workflow_steps
    });
}
