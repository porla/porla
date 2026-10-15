#pragma once

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/log/trivial.hpp>

#include "error.hpp"
#include "method.hpp"

namespace porla::Rpc
{
    // Per-call context, for method functions that need more than the request.
    class Call
    {
    public:
        Call(Auth::Context caller, ResponseWriterHandle writer)
            : m_caller(std::move(caller))
            , m_writer(std::move(writer))
        {
        }

        // Who is calling.
        const Auth::Context& Caller() const
        {
            return m_caller;
        }

        void Header(const std::string& name, const std::string& value)
        {
            m_writer->Header(name, value);
        }

    private:
        Auth::Context        m_caller;
        ResponseWriterHandle m_writer;
    };

    // Runs a method function as a coroutine on the given executor. Build it with MakeMethod.
    template<typename TReq>
    class FunctionMethod : public Method
    {
    public:
        // The adapted function: makes the call and writes the reply.
        using Fn = std::function<boost::asio::awaitable<void>(const TReq&, Call&, const ResponseWriterHandle&)>;

        FunctionMethod(boost::asio::any_io_executor executor, Fn fn)
            : m_executor(std::move(executor))
            , m_fn(std::move(fn))
        {
        }

        // Who may call is decided by the kinds passed to JsonRpc::Register.
        bool CanInvoke(const Auth::Context&) override
        {
            return true;
        }

        void Invoke(const Auth::Context& caller, const nlohmann::json& body, ResponseWriterHandle writer) override
        {
            // Decode before starting the coroutine, so bad params fail exactly as before.
            auto req = body.get<TReq>();

            boost::asio::co_spawn(
                m_executor,
                Run(m_fn, std::move(req), Call(caller, writer), writer),
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
                        BOOST_LOG_TRIVIAL(error) << "Unhandled exception in RPC method: " << ex.what();
                    }
                    catch (...)
                    {
                        BOOST_LOG_TRIVIAL(error) << "Unhandled unknown exception in RPC method";
                    }

                    writer->Error(-32603, "Internal error");
                });
        }

    private:
        // Owns the request and the call for the whole coroutine; an Rpc::Error becomes the reply.
        static boost::asio::awaitable<void> Run(Fn& fn, TReq req, Call call, ResponseWriterHandle writer)
        {
            try
            {
                co_await fn(req, call, writer);
            }
            catch (const Error& err)
            {
                writer->Error(err.code, err.what(), err.data);
            }
        }

        boost::asio::any_io_executor m_executor;
        Fn                           m_fn;
    };

    namespace Detail
    {
        template<typename T>
        struct AwaitableResult;

        template<typename T>
        struct AwaitableResult<boost::asio::awaitable<T>>
        {
            using type = T;
        };

        template<typename TReq, typename Fn>
        auto Invoked()
        {
            if constexpr (std::is_invocable_v<Fn&, const TReq&, Call&>)
                return std::type_identity<std::invoke_result_t<Fn&, const TReq&, Call&>>{};
            else
                return std::type_identity<std::invoke_result_t<Fn&, const TReq&>>{};
        }
    }

    // Makes a method from a coroutine function:
    //
    //   awaitable<void> fn(const TReq& req[, Call& call])  replies null
    //   awaitable<T>    fn(const TReq& req[, Call& call])  replies T (anything with a to_json)
    //
    // A thrown Rpc::Error replies with its code, message and data; any other exception is
    // logged and replies -32603. `call` gives the caller and lets the function set headers.
    template<typename TReq, typename Fn>
    std::shared_ptr<Method> MakeMethod(boost::asio::any_io_executor executor, Fn&& fn)
    {
        constexpr bool with_call = std::is_invocable_v<Fn&, const TReq&, Call&>;

        using Awaitable = typename decltype(Detail::Invoked<TReq, Fn>())::type;
        using Result    = typename Detail::AwaitableResult<Awaitable>::type;

        return std::make_shared<FunctionMethod<TReq>>(
            std::move(executor),
            [fn = std::forward<Fn>(fn)](const TReq& req, Call& call, const ResponseWriterHandle& writer) mutable
                -> boost::asio::awaitable<void>
            {
                if constexpr (std::is_void_v<Result>)
                {
                    if constexpr (with_call) co_await std::invoke(fn, req, call);
                    else                     co_await std::invoke(fn, req);

                    writer->Ok();
                }
                else
                {
                    if constexpr (with_call) writer->Ok(co_await std::invoke(fn, req, call));
                    else                     writer->Ok(co_await std::invoke(fn, req));
                }
            });
    }
}
