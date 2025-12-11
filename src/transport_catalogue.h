// transport_catalogue.h

#pragma once

#include "domain.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace transport_catalogue {

using namespace domain;

class TransportCatalogue {
public:
    void AddRoute(const std::string name,
                  std::vector<std::string_view> stops, bool is_roundtrip);

    void AddStop(const std::string name, const geo::Coordinates coordinates,
                 const std::unordered_map<std::string_view, size_t>
                     length_data);

    std::optional<Stop*> FindStop(std::string_view name) const;

    geo::Coordinates GetStopCoordinates(std::string_view name) const;

    std::set<std::string_view> FindBusesOnStop(std::string_view name) const;

    std::optional<Route> FindRoute(std::string_view name) const;

    RouteStats GetRouteStats(std::string_view name) const;

    void SetLengthFromTo(std::string_view from, std::string_view to,
                         size_t length);

    size_t GetLengthFromTo(std::string_view from,
                           std::string_view to) const;

    size_t GetAllStopsCount() const;

    const std::unordered_map<std::string_view, Stop*>& GetStopNameToStop()
        const;

    const std::deque<Route>& GetRoutes() const;

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::deque<Route> routes_;
    std::unordered_map<std::string_view, Route*> routename_to_route_;
    std::unordered_map<std::pair<Stop*, Stop*>, size_t, HasherPairPtr>
        length_to_stops_;

    double GetRouteDistance(std::string_view name) const;
    size_t GetRouteLength(std::string_view name) const;
    size_t GetStopsCount(std::string_view name) const;
    size_t GetUniqueStopsCount(std::string_view name) const;
    void AddLentghToStop(
        std::string_view name,
        const std::unordered_map<std::string_view, size_t> length_data);
};

} // namespace transport_catalogue