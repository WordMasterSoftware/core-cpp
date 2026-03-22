#include <cstdlib>
#include <iostream>
#include <string>

#include <httplib.h>
#include <nlohmann/json.hpp>

namespace {

using json = nlohmann::json;

constexpr int kPort = 8080;

json make_error_response(const std::string& message) {
    return json{
        {"error", message}
    };
}

}  // namespace

int main() {
    httplib::Server server;

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        json body = {
            {"message", "C++ backend is running"},
            {"usage", "POST /add with JSON: {\"a\": 1, \"b\": 2}"}
        };
        res.set_content(body.dump(), "application/json");
    });

    server.Post("/add", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const json request_body = json::parse(req.body);

            if (!request_body.contains("a") || !request_body.contains("b")) {
                res.status = 400;
                res.set_content(
                    make_error_response("Request JSON must contain numeric fields 'a' and 'b'.").dump(),
                    "application/json"
                );
                return;
            }

            if (!request_body["a"].is_number() || !request_body["b"].is_number()) {
                res.status = 400;
                res.set_content(
                    make_error_response("Fields 'a' and 'b' must be numbers.").dump(),
                    "application/json"
                );
                return;
            }

            const double a = request_body["a"].get<double>();
            const double b = request_body["b"].get<double>();

            json response_body = {
                {"result", a + b}
            };

            res.set_content(response_body.dump(), "application/json");
        } catch (const json::parse_error&) {
            res.status = 400;
            res.set_content(
                make_error_response("Invalid JSON body.").dump(),
                "application/json"
            );
        }
    });

    std::cout << "Server listening on http://0.0.0.0:" << kPort << '\n';

    if (!server.listen("0.0.0.0", kPort)) {
        std::cerr << "Failed to start server on port " << kPort << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
