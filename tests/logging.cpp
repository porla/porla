#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <gtest/gtest.h>

namespace
{
    class QuietLogs : public ::testing::Environment
    {
    public:
        void SetUp() override
        {
            boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::fatal);
        }
    };

    [[maybe_unused]] auto* const quiet_logs = ::testing::AddGlobalTestEnvironment(new QuietLogs);
}
