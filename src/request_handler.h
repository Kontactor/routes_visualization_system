// request_handler.h

#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

#include <deque>
#include <set>
#include <string>
#include <vector>

namespace request_handler {

class RequestHandler {
public:
    explicit RequestHandler(
        const transport_catalogue::TransportCatalogue& db,
        const map_renderer::MapRenderer& renderer);

    std::optional<domain::RouteStats> GetRouteStat(
        const std::string_view& bus_name) const;

    std::optional<domain::Route> GetRoute(
        const std::string_view& bus_name) const;

    const std::set<std::string_view> GetBusesByStop(
        const std::string_view& stop_name) const;

    std::optional<domain::Stop*> GetStop(
        const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

    const transport_catalogue::TransportCatalogue& GetTransportCatalogue()
        const;

private:
    const transport_catalogue::TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;

    void RenderRouteLine(
        svg::Document& document,
        const map_renderer::SphereProjector& projector,
        std::vector<std::string_view> sorted_routes) const;

    void RenderRouteName(
        svg::Document& document,
        const map_renderer::SphereProjector& projector,
        std::vector<std::string_view> sorted_routes) const;

    void RenderStopCircle(
        svg::Document& document,
        const map_renderer::SphereProjector& projector,
        std::set<std::string_view> sorted_stops) const;

    void RenderStopName(
        svg::Document& document,
        const map_renderer::SphereProjector& projector,
        std::set<std::string_view> sorted_stops) const;
};

} // namespace request_handler