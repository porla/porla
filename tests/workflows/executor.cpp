#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../inmemorysession.hpp"

#include "../../src/workflows/action.hpp"
#include "../../src/workflows/actionfactory.hpp"
#include "../../src/workflows/executor.hpp"
#include "../../src/workflows/workflow.hpp"

#include "../../src/workflows/actions/log.hpp"

using porla::Workflows::ActionCallback;
using porla::Workflows::ActionFactory;
using porla::Workflows::ActionParams;
using porla::Workflows::Executor;
using porla::Workflows::Workflow;

using porla::Workflows::Actions::Log;

struct MockAction : public porla::Workflows::Action
{
    MOCK_METHOD(void, Invoke, (const ActionParams& params, std::shared_ptr<ActionCallback> callback));
};

class ExecutorTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        session = std::make_unique<InMemorySession>();
    }

    auto LoadWorkflow(const std::string& yaml, const std::shared_ptr<MockAction>& mock_action)
    {
        return std::make_unique<Executor>(porla::Workflows::ExecutorOptions{
            .session = *session,
            .workflows = {
                Workflow::LoadFromYaml(yaml)
            },
            .action_factory = std::make_shared<porla::Workflows::ActionFactory>(
                std::map<std::string, std::function<std::shared_ptr<porla::Workflows::Action>()>>{
                    {"log", []() { return std::make_shared<Log>(); }},
                    {"mock", [&]() { return mock_action; }}
                })
        });
    }

    boost::asio::io_context io;
    std::unique_ptr<InMemorySession> session;
};

TEST_F(ExecutorTests, OnTriggerExecutesCompleteWorkflow)
{
    const auto mock_action = std::make_shared<MockAction>();
    const auto executor = LoadWorkflow(R"(
on: torrent_added

steps:
  - uses: log
    with:
      message: Hello ${{ torrent.name }}
  - uses: mock
)", mock_action);

    EXPECT_CALL(*mock_action, Invoke)
        .Times(1)
        .WillOnce(
            [](const ActionParams& params, const std::shared_ptr<ActionCallback>& callback)
            {
                EXPECT_EQ(params.Render("steps[0].rendered_message", true), "Hello test-torrent");
                callback->Complete({});
            });

    lt::torrent_status ts;
    ts.name = "test-torrent";

    session->m_torrentAdded(ts);
    io.run();
}

TEST_F(ExecutorTests, OnTriggerWithFalsyConditionSkipsWorkflow)
{
    const auto mock_action = std::make_shared<MockAction>();
    const auto executor = LoadWorkflow(R"(
on: torrent_added
if: torrent.name == "invalid"
steps:
  - uses: mock
)", mock_action);

    EXPECT_CALL(*mock_action, Invoke)
        .Times(0);

    lt::torrent_status ts;
    ts.name = "test-torrent";

    session->m_torrentAdded(ts);
    io.run();
}

TEST_F(ExecutorTests, OnTriggerWithTruthyConditionRunsWorkflow)
{
    const auto mock_action = std::make_shared<MockAction>();
    const auto executor = LoadWorkflow(R"(
on: torrent_added
if: torrent.name.includes("t-t")
steps:
  - uses: mock
)", mock_action);

    EXPECT_CALL(*mock_action, Invoke)
        .Times(1);

    lt::torrent_status ts;
    ts.name = "test-torrent";

    session->m_torrentAdded(ts);
    io.run();
}
