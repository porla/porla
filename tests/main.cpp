#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <gtest/gtest.h>

class Env : public ::testing::Environment
{
public:
    ~Env() override = default;
    void SetUp() override
    {
        boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::fatal);
    }
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new Env());
    return RUN_ALL_TESTS();
}
