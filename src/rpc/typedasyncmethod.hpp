#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/log/trivial.hpp>

#include "method.hpp"

namespace porla::Rpc
{
    template<typename TReq, typename TRes>
    class TypedAsyncMethod : public Method
    {
    public:
        explicit TypedAsyncMethod(boost::asio::any_io_executor executor)
            : m_executor(std::move(executor))
        {
        }

        void Invoke(const nlohmann::json& body, ResponseWriterHandle writer) final
        {
            boost::asio::co_spawn(
                m_executor,
                ExecuteAsync(body.get<TReq>(), writer),
                [writer](std::exception_ptr e)
                {
                    if (!e)
                    {
                        return;
                    }

                    try
                    {
                        std::rethrow_exception(e);
                    }
                    catch (const std::exception& ex)
                    {
                        BOOST_LOG_TRIVIAL(error) << "Unhandled exception in async RPC method: " << ex.what();
                    }
                    catch (...)
                    {
                        BOOST_LOG_TRIVIAL(error) << "Unhandled unknown exception in async RPC method";
                    }

                    writer->Error(-32603, "Internal error");
                });
        }

    protected:
        virtual boost::asio::awaitable<void> ExecuteAsync(TReq req, ResponseWriterHandle writer) = 0;

    private:
        boost::asio::any_io_executor m_executor;
    };
}
