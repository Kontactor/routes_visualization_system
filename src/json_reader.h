// json_reader.h

#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

#include <sstream>
#include <variant>

namespace json_reader {

struct Response {
    std::string status;
    std::string data_type;
    std::string data;
};

class JsonReader {
public:
    explicit JsonReader(std::istream& input)
        : doc_(json::Load(input))
    {
    }

    transport_catalogue::TransportCatalogue ReadTransportCatalogue()
        const;

    map_renderer::RenderSettings FillRenderSettings() const;

    domain::RouterSettings FillRouterSettings() const;

    Response GenerateResponses(
        request_handler::RequestHandler& handler) const;

private:
    const json::Document doc_;

    std::optional<domain::RouteInfo> GetRouteInfo(
        std::string_view start, std::string_view end,
        router::TransportRouter& routing,
        request_handler::RequestHandler& handler) const;
};

} // namespace json_reader