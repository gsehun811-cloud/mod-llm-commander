/* Modified for standalone mod-llm-commander; compatibility audit 2026-11-03. */
/*
 * Modified distribution: mod-playerbots-llm-commander.
 * This file differs from bigr00/mod-llm-playerbots c0ba652.
 * Modified version published on 2026-10-01.
 * Modification notice added on 2026-10-01.
 * Publication date does not identify every historical edit date.
 * Original copyright and license notices remain in effect.
 */
/*
 * This file is part of the LLM bots feature of mod-playerbots (AzerothCore).
 */

#include "CommanderHttpsClient.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <vector>
#include <cstdlib>
#include <filesystem>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = net::ip::tcp;

namespace LlmCommander
{

HttpResponse HttpsPostJson(std::string const& host, std::string const& port,
    std::string const& path, std::string const& body,
    std::vector<std::pair<std::string, std::string>> const& headers,
    uint32_t timeoutMs)
{
    HttpResponse result;
    try
    {
        net::io_context ioc;
        ssl::context ctx(ssl::context::tls_client);
        beast::error_code trustError;
        ctx.set_default_verify_paths(trustError);
        if (char const* caFile = std::getenv("LLM_COMMANDER_CA_FILE"))
            ctx.load_verify_file(caFile);
        else if (std::filesystem::exists("ca-bundle.crt"))
            ctx.load_verify_file("ca-bundle.crt");
        ctx.set_verify_mode(ssl::verify_peer);

        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
        stream.set_verify_callback(ssl::host_name_verification(host));
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
        {
            result.error = "failed to set TLS hostname";
            return result;
        }

        http::request<http::string_body> req(http::verb::post, path, 11);
        req.set(http::field::host, host + (port == "443" ? "" : ":" + port));
        req.set(http::field::content_type, "application/json");
        req.set(http::field::user_agent, "mod-llm-commander/0.1");
        for (auto const& [key, value] : headers)
            req.set(key, value);
        req.body() = body;
        req.prepare_payload();

        beast::flat_buffer buffer;
        http::response_parser<http::string_body> parser;
        parser.body_limit(2 * 1024 * 1024);
        net::steady_timer deadline(ioc);
        bool finished = false;
        auto finish = [&](beast::error_code ec)
        {
            if (finished)
                return;
            finished = true;
            deadline.cancel();
            resolver.cancel();
            beast::error_code ignored;
            beast::get_lowest_layer(stream).socket().cancel(ignored);
            beast::get_lowest_layer(stream).socket().close(ignored);
            if (ec)
                result.error = ec.message();
            else
            {
                result.ok = true;
                result.status = parser.get().result_int();
                result.body = parser.get().body();
            }
        };

        // One wall-clock deadline covers DNS, connect, TLS, write and read.
        deadline.expires_after(std::chrono::milliseconds(timeoutMs));
        deadline.async_wait([&](beast::error_code ec)
        {
            if (!ec)
                finish(net::error::timed_out);
        });
        resolver.async_resolve(host, port, [&](beast::error_code ec, tcp::resolver::results_type results)
        {
            if (finished)
                return;
            if (ec)
                return finish(ec);
            beast::get_lowest_layer(stream).async_connect(results,
                [&](beast::error_code connectError, tcp::resolver::results_type::endpoint_type)
            {
                if (finished)
                    return;
                if (connectError)
                    return finish(connectError);
                stream.async_handshake(ssl::stream_base::client, [&](beast::error_code handshakeError)
                {
                    if (finished)
                        return;
                    if (handshakeError)
                        return finish(handshakeError);
                    http::async_write(stream, req, [&](beast::error_code writeError, std::size_t)
                    {
                        if (finished)
                            return;
                        if (writeError)
                            return finish(writeError);
                        http::async_read(stream, buffer, parser, [&](beast::error_code readError, std::size_t)
                        {
                            finish(readError);
                        });
                    });
                });
            });
        });
        ioc.run();
    }
    catch (std::exception const& e)
    {
        result.ok = false;
        result.error = e.what();
    }
    return result;
}

} // namespace LlmCommander
