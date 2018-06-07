#include <iostream>

#include <restinio/all.hpp>

namespace rest = restinio;

// Create request handler.
rest::request_handling_status_t handler(rest::request_handle_t req)
{
    if (rest::http_method_get() == req->header().method()) {
        std::ostringstream sout;
        sout << "GET request to '" << req->header().request_target() << "'\n";

        // Query params.
        const auto qp = rest::parse_query(req->header().query());

        if (0 == qp.size()) {
            sout << "No query parameters.";
        } else {
            sout << "Query params (" << qp.size() << "):\n";

            for (const auto p : qp) {
                sout << "'" << p.first << "' => " << p.second << "'\n";
            }
        }

        if (qp.has("debug") && qp["debug"] == "true") {
            std::cout << sout.str() << std::endl;
        }

        req->create_response()
            .append_header(rest::http_field::server, "OPAQ prediction server")
            .append_header_date_field()
            .append_header(rest::http_field::content_type, "text/plain; charset=utf-8")
            .set_body(sout.str())
            .done();

        return rest::request_accepted();
    }

    return rest::request_rejected();
}

int main()
{
    try {
        rest::run(
            rest::on_thread_pool(std::thread::hardware_concurrency())
                .port(8080)
                .address("localhost")
                .request_handler(handler));
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
