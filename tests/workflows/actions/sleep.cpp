#include <gtest/gtest.h>

#include "../../../src/workflows/action.hpp"
#include "../../../src/workflows/actions/sleep.hpp"

using porla::Workflows::ActionCallback;
using porla::Workflows::ActionParams;
using porla::Workflows::Actions::Sleep;

struct FuncActionCallback : public ActionCallback
{
    explicit FuncActionCallback(const std::function<void(const nlohmann::json&)>& cb)
        : m_cb(cb)
    {
    }

    void Complete(const nlohmann::json& output) override
    {
        m_cb(output);
    }

private:
    std::function<void(const nlohmann::json&)> m_cb;
};

struct MockActionParams : public ActionParams
{
    explicit MockActionParams(const nlohmann::json& input)
        : m_input(input)
    {
    }

    [[nodiscard]] nlohmann::json Input() const override
    {
        return m_input;
    }

    [[nodiscard]] std::string RenderValues(const std::string& text) const override
    {
        return text;
    }

private:
    nlohmann::json m_input;
};

class SleepTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        sleep = std::make_unique<Sleep>(io);
    }

    boost::asio::io_context io;
    std::unique_ptr<Sleep> sleep;
};

TEST_F(SleepTests, Invoke_WithValidParams_Returns)
{
    const MockActionParams p{{
        {"timeout", 100}
    }};

    bool callback_called = false;

    const auto cb = [&callback_called](const nlohmann::json& output)
    {
        callback_called = true;

        // Sleep does not produce any output
        EXPECT_EQ(output, nullptr);
    };

    sleep->Invoke(p, std::make_shared<FuncActionCallback>(cb));
    io.run();

    EXPECT_TRUE(callback_called);
}
