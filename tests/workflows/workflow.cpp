#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <libtorrent/torrent_status.hpp>

#include "../../src/workflows/action.hpp"
#include "../../src/workflows/actionfactory.hpp"
#include "../../src/workflows/step.hpp"
#include "../../src/workflows/torrentcontextprovider.hpp"
#include "../../src/workflows/workflow.hpp"

#include "../../src/workflows/actions/log.hpp"

using porla::Workflows::ActionCallback;
using porla::Workflows::ActionFactory;
using porla::Workflows::ActionParams;
using porla::Workflows::Step;
using porla::Workflows::TorrentContextProvider;
using porla::Workflows::Workflow;
using porla::Workflows::WorkflowOptions;

using porla::Workflows::Action;
using porla::Workflows::Actions::Log;

struct MockAction : public porla::Workflows::Action
{
    MOCK_METHOD(void, Invoke, (const ActionParams& params, std::shared_ptr<ActionCallback> callback));
};

class WorkflowTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mock_action = std::make_shared<MockAction>();

        action_factory = std::make_unique<ActionFactory>(std::map<std::string, std::function<std::shared_ptr<Action>()>>{
            {"log", []() { return std::make_shared<Log>(); }},
            {"mock", [this]() { return mock_action; }}
        });
    }

    std::unique_ptr<ActionFactory> action_factory;
    std::shared_ptr<MockAction> mock_action;
};

TEST_F(WorkflowTests, HasWorkingOutputs)
{
    Workflow w{WorkflowOptions{
        .on = {},
        .steps = {
            Step{
                .uses = "log",
                .with = {
                    {"message", "Hello ${{ torrent.name }}"}
                }
            },
            Step{
                .uses = "mock"
            }
        }
    }};

    libtorrent::torrent_status ts;
    ts.name = "test-torrent";

    EXPECT_CALL(*mock_action, Invoke)
        .Times(1)
        .WillOnce(
            [](const ActionParams& params, const std::shared_ptr<ActionCallback>& callback)
            {
                EXPECT_EQ(params.Render("steps[0].rendered_message", true), "Hello test-torrent");
                callback->Complete({});
            });

    w.Execute(*action_factory, {
        {"torrent", std::make_shared<TorrentContextProvider>(ts)}
    });
}
