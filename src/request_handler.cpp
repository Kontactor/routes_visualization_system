// request_handler.cpp

#include "request_handler.h"

namespace request_handler {

std::vector<std::string_view> SortRoutes(
    const std::deque<domain::Route>& routes)
{
    std::vector<std::string_view> sorted_routes;
    for (const auto& route : routes) {
        sorted_routes.push_back(route.name);
    }
    std::sort(sorted_routes.begin(), sorted_routes.end());

    return sorted_routes;
}

std::vector<geo::Coordinates> CollectCoordinates(
    const std::deque<domain::Route>& routes)
{
    std::vector<geo::Coordinates> coordinates;

    for (const auto& route : routes) {
        for (const auto& stop : route.stops) {
            coordinates.push_back(stop->coordinates);
        }
    }
    return coordinates;
}

std::set<std::string_view> CollectStopsName(
    const std::deque<domain::Route>& routes)
{
    std::set<std::string_view> stops_name;
    for (const auto& route : routes) {
        for (const auto& stop : route.stops) {
            stops_name.insert(stop->name);
        }
    }
    return stops_name;
}

RequestHandler::RequestHandler(
    const transport_catalogue::TransportCatalogue& db,
    const map_renderer::MapRenderer& renderer)
    : db_(db)
    , renderer_(renderer)
{
}

std::optional<domain::RouteStats> RequestHandler::GetRouteStat(
    const std::string_view& bus_name) const
{
    if (db_.FindRoute(bus_name).has_value()) {
        return db_.GetRouteStats(bus_name);
    }
    return std::nullopt;
}

std::optional<domain::Route> RequestHandler::GetRoute(
    const std::string_view& bus_name) const
{
    if (db_.FindRoute(bus_name).has_value()) {
        return db_.FindRoute(bus_name);
    }
    return std::nullopt;
}

const std::set<std::string_view> RequestHandler::GetBusesByStop(
    const std::string_view& stop_name) const
{
    return db_.FindBusesOnStop(stop_name);
}

std::optional<domain::Stop*> RequestHandler::GetStop(
    const std::string_view& stop_name) const
{
    return db_.FindStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const
{
    svg::Document document;

    std::vector<geo::Coordinates> geo_coord =
        CollectCoordinates(db_.GetRoutes());

    map_renderer::SphereProjector projector(
        geo_coord.begin(), geo_coord.end(),
        renderer_.GetRenderSettings().width,
        renderer_.GetRenderSettings().height,
        renderer_.GetRenderSettings().padding);

    std::vector<std::string_view> sorted_routes =
        SortRoutes(db_.GetRoutes());
    std::set<std::string_view> sorted_stops =
        CollectStopsName(db_.GetRoutes());

    RenderRouteLine(document, projector, sorted_routes);
    RenderRouteName(document, projector, sorted_routes);
    RenderStopCircle(document, projector, sorted_stops);
    RenderStopName(document, projector, sorted_stops);

    return document;
}

const transport_catalogue::TransportCatalogue&
RequestHandler::GetTransportCatalogue() const
{
    return db_;
}

void RequestHandler::RenderRouteLine(
    svg::Document& document,
    const map_renderer::SphereProjector& projector,
    std::vector<std::string_view> sorted_routes) const
{
    size_t route_counter = 0;
    for (const auto& route_name : sorted_routes) {
        auto route = db_.FindRoute(route_name).value();
        if (!route.stops.empty()) {
            std::vector<svg::Point> projectet_coord;
            for (const auto& stop : route.stops) {
                projectet_coord.push_back(projector(stop->coordinates));
            }

            document.Add(
                renderer_.RenderRouteLine(projectet_coord, route_counter));
            ++route_counter;
        }
    }
}

void RequestHandler::RenderRouteName(
    svg::Document& document,
    const map_renderer::SphereProjector& projector,
    std::vector<std::string_view> sorted_routes) const
{
    size_t route_counter = 0;
    for (const auto& route_name : sorted_routes) {
        auto route = db_.FindRoute(route_name).value();
        if (!route.stops.empty()) {
            svg::Point projectet_coord_f =
                projector(route.stops.front()->coordinates);
            document.Add(renderer_.RenderRouteNameSubstrate(
                route_name, projectet_coord_f));
            document.Add(renderer_.RenderRouteName(route_name,
                                                   projectet_coord_f,
                                                   route_counter));

            if (!route.is_roundtrip &&
                route.stops.at(route.stops.size() / 2) !=
                    route.stops.front()) {
                svg::Point projectet_coord_l =
                    projector(route.stops.at(route.stops.size() / 2)
                                  ->coordinates);
                document.Add(renderer_.RenderRouteNameSubstrate(
                    route_name, projectet_coord_l));
                document.Add(renderer_.RenderRouteName(route_name,
                                                       projectet_coord_l,
                                                       route_counter));
            }
        }
        ++route_counter;
    }
}

void RequestHandler::RenderStopCircle(
    svg::Document& document,
    const map_renderer::SphereProjector& projector,
    std::set<std::string_view> sorted_stops) const
{
    for (const auto& stop_name : sorted_stops) {
        auto stop = db_.FindStop(stop_name).value();
        document.Add(
            renderer_.RenderStopCircle(projector(stop->coordinates)));
    }
}

void RequestHandler::RenderStopName(
    svg::Document& document,
    const map_renderer::SphereProjector& projector,
    std::set<std::string_view> sorted_stops) const
{
    for (const auto& stop_name : sorted_stops) {
        auto stop = db_.FindStop(stop_name).value();
        document.Add(renderer_.RenderStopNameSubstrate(
            stop->name, projector(stop->coordinates)));
        document.Add(renderer_.RenderStopName(stop->name,
                                              projector(stop->coordinates)));
    }
}

} // namespace request_handler