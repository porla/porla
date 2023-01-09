#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../inmemorysession.hpp"

#include "../../src/workflows/action.hpp"
#include "../../src/workflows/actionfactory.hpp"
#include "../../src/workflows/executor.hpp"
#include "../../src/workflows/workflow.hpp"

using porla::Workflows::ActionCallback;
using porla::Workflows::ActionFactory;
using porla::Workflows::ActionParams;
using porla::Workflows::Executor;
using porla::Workflows::Workflow;

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

        auto workflow = Workflow::LoadFromYaml(R"(
on: torrent_added

steps:
  - uses: mock
  - uses: mock
)");

        mock_action = std::make_shared<MockAction>();

        executor = std::make_unique<Executor>(porla::Workflows::ExecutorOptions{
            .session = *session,
            .workflows = {
                workflow
            },
            .action_factory = std::make_shared<porla::Workflows::ActionFactory>(
                std::map<std::string, std::function<std::shared_ptr<porla::Workflows::Action>()>>{
                    {"mock", [this]() { return mock_action; }}
                })
        });
    }

    boost::asio::io_context io;
    std::shared_ptr<MockAction> mock_action;
    std::unique_ptr<Executor> executor;
    std::unique_ptr<InMemorySession> session;
};

TEST_F(ExecutorTests, OnTriggerExecutesCompleteWorkflow)
{
    EXPECT_CALL(*mock_action, Invoke)
        .Times(2)
        .WillRepeatedly(
            [](const ActionParams& params, const std::shared_ptr<ActionCallback>& callback)
            {
                callback->Complete({});
            });

    session->m_torrentAdded(lt::torrent_status{});
    io.run();
}
