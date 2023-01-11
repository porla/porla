#include "workflow.hpp"

#include <fstream>
#include <regex>
#include <utility>
#include <vector>

#include <boost/log/trivial.hpp>
#include <libjsonnet++.h>
#include <ryml/ryml.hpp>

#include "action.hpp"
#include "actionfactory.hpp"
#include "contextprovider.hpp"
#include "step.hpp"
#include "textrenderer.hpp"

using porla::Workflows::Action;
using porla::Workflows::ActionCallback;
using porla::Workflows::ActionFactory;
using porla::Workflows::ContextProvider;
using porla::Workflows::Step;
using porla::Workflows::TextRenderer;
using porla::Workflows::Workflow;
using porla::Workflows::WorkflowOptions;

struct StepInstance
{
    std::shared_ptr<Action> action;
    Step                    step;
};

class StepContextProvider : public ContextProvider
{
public:
    void AddOutput(const nlohmann::json& j)
    {
        m_outputs.push_back(j);
    }

    nlohmann::json Value() override
    {
        return m_outputs;
    }

    nlohmann::json m_outputs;
};

class SimpleActionParams : public porla::Workflows::ActionParams
{
public:
    explicit SimpleActionParams(nlohmann::json input, const std::function<nlohmann::json(std::string, bool)>& renderer)
        : m_input(std::move(input))
        , m_renderer(renderer)
    {
    }

    [[nodiscard]] nlohmann::json Input() const override
    {
        return m_input;
    }

    [[nodiscard]] nlohmann::json Render(const std::string& text, bool raw_expression) const override
    {
        return m_renderer(text, raw_expression);
    }

private:
    nlohmann::json m_input;
    std::function<nlohmann::json(std::string, bool)> m_renderer;
};

class LoopingWorkflowRunner : public ActionCallback, public std::enable_shared_from_this<LoopingWorkflowRunner>
{
public:
    explicit LoopingWorkflowRunner(
        const std::map<std::string, std::shared_ptr<ContextProvider>>& contexts,
        const std::vector<StepInstance>& step_instances)
        : m_contexts(contexts)
        , m_step_context_provider(std::make_shared<StepContextProvider>())
        , m_step_instances(step_instances)
        , m_current_index(0)
    {
        m_contexts.insert({"steps", m_step_context_provider});
    }

    void Complete(const nlohmann::json& j) override
    {
        // This is the instance that completed. Set its output.
        auto& instance = m_step_instances.at(m_current_index);

        m_current_index++;
        m_step_context_provider->AddOutput(j);

        if (m_current_index >= m_step_instances.size())
        {
            return;
        }

        Run();
    }

    void Run()
    {
        const auto& instance = m_step_instances.at(m_current_index);

        SimpleActionParams sap{
            instance.step.with,
            [_this = shared_from_this()](const std::string& text, bool raw_expression)
            {
                TextRenderer renderer{_this->m_contexts};
                return renderer.Render(text, raw_expression);
            }};

        try
        {
            instance.action->Invoke(sap, shared_from_this());
        }
        catch (const std::exception& ex)
        {
            BOOST_LOG_TRIVIAL(error) << "Error when invoking action " << instance.step.uses << ": " << ex.what();
        }
    }

private:
    std::map<std::string, std::shared_ptr<ContextProvider>> m_contexts;
    std::shared_ptr<StepContextProvider> m_step_context_provider;
    std::vector<StepInstance> m_step_instances;
    int m_current_index;
};

Workflow::Workflow(const WorkflowOptions &opts)
    : m_on(opts.on)
    , m_steps(opts.steps)
{
}

Workflow::~Workflow() = default;

void Workflow::Execute(
    const ActionFactory &action_factory,
    const std::map<std::string, std::shared_ptr<ContextProvider>> &contexts)
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

std::unordered_set<std::string> Workflow::On()
{
    return m_on;
}

std::shared_ptr<Workflow> Workflow::LoadFromFile(const std::filesystem::path& workflow_file)
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

    return LoadFromYaml(workflow_file_buffer.data());
}

std::shared_ptr<Workflow> Workflow::LoadFromYaml(const std::string& yaml)
{
    ryml::Tree wf = ryml::parse_in_arena(ryml::to_csubstr(yaml.c_str()));

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

            if (step.has_child("with"))
            {
                for (const ryml::NodeRef &with_item: step["with"].children())
                {
                    const auto key = with_item.key();
                    const auto val = with_item.val();

                    std::string key_str(key.data(), key.size());
                    std::string val_str(val.data(), val.size());

                    if (val.is_integer())
                    {
                        with.insert({key_str, std::stoi(val_str)});
                    }
                    else
                    {
                        with.insert({key_str, val_str});
                    }
                }
            }

            workflow_steps.emplace_back(Step{
                .uses = std::string(uses_val.data(), uses_val.size()),
                .with = with
            });
        }
    }

    return std::make_shared<Workflow>(WorkflowOptions{
        .on    = {on},
        .steps = workflow_steps
    });
}
