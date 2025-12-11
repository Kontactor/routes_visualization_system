// transport_catalogue.cpp

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddRoute(const std::string name,
                                  std::vector<std::string_view> stops,
                                  bool is_roundtrip)
{
    std::vector<Stop*> tc_stops;
    for (std::string_view stop : stops) {
        std::unordered_map<std::string_view, Stop*>::iterator pos =
            stopname_to_stop_.find(stop);
        tc_stops.push_back(pos->second);
    }
    routes_.push_back({std::move(name), std::move(tc_stops), is_roundtrip});
    routename_to_route_[routes_.back().name] = &routes_.back();
}

void TransportCatalogue::AddStop(
    const std::string name, const geo::Coordinates coordinates,
    const std::unordered_map<std::string_view, size_t> length_data)
{
    if (stopname_to_stop_.count(name)) {
        stopname_to_stop_.at(name)->coordinates = coordinates;
    } else {
        stops_.push_back({std::move(name), coordinates});
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }
    if (!length_data.empty()) {
        AddLentghToStop(name, length_data);
    }
}

std::optional<Stop*> TransportCatalogue::FindStop(std::string_view name) const
{
    auto it = stopname_to_stop_.find(name);
    if (it != stopname_to_stop_.end()) {
        return it->second;
    }
    return std::nullopt;
}

geo::Coordinates TransportCatalogue::GetStopCoordinates(
    std::string_view name) const
{
    return stopname_to_stop_.at(name)->coordinates;
}

std::set<std::string_view> TransportCatalogue::FindBusesOnStop(
    std::string_view name) const
{
    std::set<std::string_view> buses;
    for (const Route& route : routes_) {
        if (std::find(route.stops.begin(), route.stops.end(),
                      stopname_to_stop_.at(name)) != route.stops.end()) {
            buses.insert(route.name);
        }
    }
    return buses;
}

std::optional<Route> TransportCatalogue::FindRoute(std::string_view name) const
{
    auto it = routename_to_route_.find(name);
    if (it != routename_to_route_.end()) {
        return *it->second;
    }
    return std::nullopt;
}

RouteStats TransportCatalogue::GetRouteStats(std::string_view name) const
{
    return {GetRouteDistance(name), GetRouteLength(name),
            GetStopsCount(name), GetUniqueStopsCount(name)};
}

void TransportCatalogue::SetLengthFromTo(std::string_view from,
                                         std::string_view to, size_t length)
{
    if (!stopname_to_stop_.count(from)) {
        AddStop(std::string(from), {91, 181},
                std::unordered_map<std::string_view, size_t>());
    }
    if (!stopname_to_stop_.count(to)) {
        AddStop(std::string(to), {91, 181},
                std::unordered_map<std::string_view, size_t>());
    }
    length_to_stops_[{stopname_to_stop_.at(from),
                      stopname_to_stop_.at(to)}] = length;
}

size_t TransportCatalogue::GetLengthFromTo(std::string_view from,
                                           std::string_view to) const
{
    size_t length = 0;
    if (length_to_stops_.count(
            {stopname_to_stop_.at(from), stopname_to_stop_.at(to)})) {
        length = length_to_stops_.at(
            {stopname_to_stop_.at(from), stopname_to_stop_.at(to)});
    } else if (length_to_stops_.count(
                   {stopname_to_stop_.at(to), stopname_to_stop_.at(from)})) {
        length = length_to_stops_.at(
            {stopname_to_stop_.at(to), stopname_to_stop_.at(from)});
    }
    return length;
}

size_t TransportCatalogue::GetAllStopsCount() const
{
    return stops_.size();
}

const std::unordered_map<std::string_view, Stop*>&
TransportCatalogue::GetStopNameToStop() const
{
    return stopname_to_stop_;
}

const std::deque<Route>& TransportCatalogue::GetRoutes() const
{
    return routes_;
}

size_t TransportCatalogue::GetStopsCount(std::string_view name) const
{
    return routename_to_route_.at(name)->stops.size();
}

size_t TransportCatalogue::GetUniqueStopsCount(std::string_view name) const
{
    std::set<std::string_view> ustops;
    for (auto i : routename_to_route_.at(name)->stops) {
        ustops.insert(i->name);
    }
    return ustops.size();
}

double TransportCatalogue::GetRouteDistance(std::string_view name) const
{
    double distance = 0;

    if (routename_to_route_.count(name)) {
        for (size_t i = 0; i < routename_to_route_.at(name)->stops.size() - 1;
             ++i) {
            distance += geo::ComputeDistance(
                routename_to_route_.at(name)->stops.at(i)->coordinates,
                routename_to_route_.at(name)->stops.at(i + 1)->coordinates);
        }
    }
    return distance;
}

void TransportCatalogue::AddLentghToStop(
    std::string_view name,
    const std::unordered_map<std::string_view, size_t> length_data)
{
    for (auto [stop, length] : length_data) {
        SetLengthFromTo(name, stop, length);
    }
}

size_t TransportCatalogue::GetRouteLength(std::string_view name) const
{
    size_t length = 0;

    if (routename_to_route_.count(name)) {
        for (size_t i = 0; i < routename_to_route_.at(name)->stops.size() - 1;
             ++i) {
            length += GetLengthFromTo(
                routename_to_route_.at(name)->stops.at(i)->name,
                routename_to_route_.at(name)->stops.at(i + 1)->name);
        }
    }
    length += GetLengthFromTo(
        routename_to_route_.at(name)->stops.front()->name,
        routename_to_route_.at(name)->stops.front()->name);
    return length;
}

} // namespace transport_catalogue