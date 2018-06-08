#include <iostream>

#include <memory>
#include <restinio/all.hpp>

#include "infra/log.h"

namespace rest = restinio;

using infra::Log;
using Router = rest::router::express_router_t<>;

template <typename ResponseType>
ResponseType initResponse(ResponseType resp)
{
    resp.append_header(rest::http_field::server, "OPAQ prediction server");
    resp.append_header_date_field();
    return resp;
};

auto createRequestHandler()
{
    auto router = std::make_unique<Router>();
    router->http_get("/", [](auto req, auto) {
        return initResponse(req->create_response())
            .append_header(rest::http_field::content_type, "text/plain; charset=utf-8")
            .set_body("OPAQ prediction server")
            .done();
    });

    router->http_get("/getprediction", [](auto req, auto) {
        // Query params.
        const auto qp = rest::parse_query(req->header().query());
        Log::info("Prediction query: date='{}' model='{}'", qp.get_param("date").value_or("None"), qp.get_param("model").value_or("None"));

        return initResponse(req->create_response())
            .append_header(rest::http_field::content_type, "text/json; charset=utf-8")
            .set_body("{ prediction-response: 'value' }")
            .done();
    });

    router->non_matched_request_handler([](auto req) {
        return req->create_response(400, "Bad Request")
            .append_header_date_field()
            .connection_close()
            .done();
    });

    return router;
}

int main()
{
    try {
        Log::addConsoleSink(Log::Colored::On);
        infra::LogRegistration logging("predictionservice");
        Log::setLevel(Log::Level::Info);

        // logging trait: rest::single_threaded_ostream_logger_t
        using traits_t = rest::traits_t<rest::asio_timer_manager_t, rest::null_logger_t, Router>;

        rest::run(rest::on_this_thread<traits_t>()
                      .port(8080)
                      .address("localhost")
                      .request_handler(createRequestHandler()));
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
