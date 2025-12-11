// domain.h

#pragma once

#include "geo.h"
#include "graph.h"

#include <string>
#include <variant>
#include <vector>

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Route {
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};

struct RouteStats {
    double geo_distance;
    size_t route_length;
    size_t stops_count;
    size_t uniquestops_count;
};

class HasherPairPtr {
public:
    size_t operator()(const std::pair<Stop*, Stop*>& pair_ptr) const;
};

struct RouterSettings {
    double bus_wait_time = 0;
    double bus_velocity = 0;
};

struct StopEdge {
    Stop* stopptr;
    double time = 0;
};

struct BusEdge {
    const Route* busptr;
    size_t span_count = 0;
    double time = 0;
};

struct StopVertexIds {
    graph::VertexId bus_wait_start;
    graph::VertexId bus_wait_end;
};

struct RouteInfo {
    double total_time = 0.;
    std::vector<std::variant<StopEdge, BusEdge>> edges;
};

} // namespace domain