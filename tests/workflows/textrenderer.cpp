#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../src/workflows/contextprovider.hpp"
#include "../../src/workflows/textrenderer.hpp"

using porla::Workflows::ContextProvider;
using porla::Workflows::TextRenderer;

class MockContextProvider : public ContextProvider
{
public:
    nlohmann::json ResolveSegments(const std::vector<std::string>& segments) override
    {
        return "foo";
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
