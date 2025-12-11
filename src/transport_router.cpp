// transport_router.cpp

#include "transport_router.h"

namespace router {

TransportRouter::TransportRouter(
    const transport_catalogue::TransportCatalogue& catalogue,
    domain::RouterSettings router_settings, size_t graph_size)
    : graph_(std::make_unique<graph::DirectedWeightedGraph<double>>(
          2 * graph_size))
    , router_settings_(router_settings)
{
    BuildRouter(catalogue);
}

void TransportRouter::BuildRouter(
    const transport_catalogue::TransportCatalogue& catalogue)
{
    SetGraph(catalogue);
    router_ = std::make_unique<graph::Router<double>>(*graph_);
}

std::optional<domain::RouteInfo> TransportRouter::GetRouteInfo(
    graph::VertexId start, graph::VertexId end) const
{
    const auto& route_info = router_->BuildRoute(start, end);
    if (route_info) {
        domain::RouteInfo result;
        result.total_time = route_info->weight;

        for (const auto edge : route_info->edges) {
            result.edges.emplace_back(GetEdge(edge));
        }
        return result;
    } else {
        return std::nullopt;
    }
}

const std::variant<domain::StopEdge, domain::BusEdge>&
TransportRouter::GetEdge(graph::EdgeId id) const
{
    return edgeid_to_edge_.at(id);
}

std::optional<domain::StopVertexIds> TransportRouter::GetVertexIdByStop(
    domain::Stop* stop) const
{
    if (stopptr_to_vertexid_.count(stop)) {
        return stopptr_to_vertexid_.at(stop);
    } else {
        return std::nullopt;
    }
}

void TransportRouter::SetGraph(
    const transport_catalogue::TransportCatalogue& catalogue)
{
    SetStopVertices(catalogue.GetStopNameToStop());
    AddEdgeToStop();
    AddEdgeToBus(catalogue);
}

void TransportRouter::SetStopVertices(
    const std::unordered_map<std::string_view, domain::Stop*>&
        stopname_to_stop_)
{
    size_t i = 0;
    for (const auto& [name, ptr] : stopname_to_stop_) {
        graph::VertexId first = i++;
        graph::VertexId second = i++;

        stopptr_to_vertexid_[ptr] = domain::StopVertexIds{first, second};
    }
}

void TransportRouter::AddEdgeToStop()
{
    for (const auto [stop, number] : stopptr_to_vertexid_) {
        graph::EdgeId id = graph_->AddEdge(
            graph::Edge<double>{number.bus_wait_start, number.bus_wait_end,
                                router_settings_.bus_wait_time});

        edgeid_to_edge_[id] =
            domain::StopEdge{stop, router_settings_.bus_wait_time};
    }
}

void TransportRouter::AddEdgeToBus(
    const transport_catalogue::TransportCatalogue& catalogue)
{
    for (const auto& route : catalogue.GetRoutes()) {
        for (auto it_1 : std::ranges::views::iota(route.stops.begin(),
                                                  route.stops.end())) {
            size_t distance = 0;
            size_t span = 0;
            for (auto it_2 : std::ranges::views::iota(std::next(it_1),
                                                      route.stops.end())) {
                distance += catalogue.GetLengthFromTo(
                    (*std::prev(it_2))->name, (*it_2)->name);
                ++span;

                graph::EdgeId id = graph_->AddEdge(graph::Edge<double>{
                    stopptr_to_vertexid_.at(*it_1).bus_wait_end,
                    stopptr_to_vertexid_.at(*it_2).bus_wait_start,
                    CalcWeight(distance)});
                edgeid_to_edge_[id] =
                    domain::BusEdge{&route, span, graph_->GetEdge(id).weight};
            }
        }
        if (!route.is_roundtrip) {
            size_t distance = 0;
            size_t span = 0;
            for (auto it_1 : std::ranges::views::iota(route.stops.rbegin(),
                                                      route.stops.rend())) {
                for (auto it_2 : std::ranges::views::iota(std::next(it_1),
                                                          route.stops.rend())) {
                    distance += catalogue.GetLengthFromTo(
                        (*std::prev(it_2))->name, (*it_2)->name);
                    ++span;

                    graph::EdgeId id = graph_->AddEdge(graph::Edge<double>{
                        stopptr_to_vertexid_.at(*it_1).bus_wait_end,
                        stopptr_to_vertexid_.at(*it_2).bus_wait_start,
                        CalcWeight(distance)});
                    edgeid_to_edge_[id] = domain::BusEdge{
                        &route, span, graph_->GetEdge(id).weight};
                }
            }
        }
    }
}

double TransportRouter::CalcWeight(size_t distance)
{
    return static_cast<double>(distance) /
           (router_settings_.bus_velocity * KILOMETER / HOUR);
}

} // namespace router