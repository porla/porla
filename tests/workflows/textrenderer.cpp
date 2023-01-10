#include <gtest/gtest.h>

#include "../../src/workflows/contextprovider.hpp"
#include "../../src/workflows/textrenderer.hpp"

using porla::Workflows::ContextProvider;
using porla::Workflows::TextRenderer;

class MockContextProvider : public ContextProvider
{
public:
    nlohmann::json Value() override
    {
        return {
            {"foo", "bar"},
            {"num", 1337 }
        };
    }
};

class TextRendererTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        renderer = std::make_unique<TextRenderer>(
            std::map<std::string, std::shared_ptr<ContextProvider>>{
                {"mock", std::make_shared<MockContextProvider>()}
            });
    }

    std::unique_ptr<TextRenderer> renderer;
};

TEST_F(TextRendererTests, Render_WithNoContextLookups_ReturnsOriginalValue)
{
    const auto rendered_value = renderer->Render("Simple text rendering");
    EXPECT_EQ(rendered_value, "Simple text rendering");
}

TEST_F(TextRendererTests, Render_WithSingleMockContextLookup_ReturnsRenderedValue)
{
    const auto rendered_value = renderer->Render("This is ${{ mock.foo }}");
    EXPECT_EQ(rendered_value, "This is bar");
}

TEST_F(TextRendererTests, Render_WithMultipleContextLookups_ReturnsCorrectValue)
{
    const auto rendered_value = renderer->Render("This is ${{ mock.foo }}: ${{ mock.num }} :)");
    EXPECT_EQ(rendered_value, "This is bar: 1337 :)");
}
