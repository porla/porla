#include <gtest/gtest.h>

#include "../../../src/workflows/action.hpp"
#include "../../../src/workflows/actions/log.hpp"

using porla::Workflows::ActionCallback;
using porla::Workflows::ActionParams;
using porla::Workflows::Actions::Log;

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

class LogTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        sleep = std::make_unique<Log>();
    }

    std::unique_ptr<Log> sleep;
};

TEST_F(LogTests, Invoke_WithValidParams_Returns)
{
    const MockActionParams p{{
         {"message", "Hello!"}
     }};

    bool callback_called = false;

    const auto cb = [&callback_called](const nlohmann::json& output)
    {
        using namespace nlohmann::literals;

        callback_called = true;
        EXPECT_EQ(output, R"({"rendered_message": "Hello!" })"_json);
    };

    sleep->Invoke(p, std::make_shared<FuncActionCallback>(cb));

    EXPECT_TRUE(callback_called);
}
