// transport_router.h

#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <ranges>
#include <variant>

namespace router {

static const uint16_t KILOMETER = 1000;
static const uint16_t HOUR = 60;

class TransportRouter {
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
                    domain::RouterSettings router_settings, size_t graph_size);

    std::optional<domain::RouteInfo> GetRouteInfo(graph::VertexId start,
                                                  graph::VertexId end) const;

    const std::variant<domain::StopEdge, domain::BusEdge>& GetEdge(
        graph::EdgeId id) const;

    std::optional<domain::StopVertexIds> GetVertexIdByStop(
        domain::Stop* stop) const;

private:
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    domain::RouterSettings router_settings_;

    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<domain::Stop*, domain::StopVertexIds>
        stopptr_to_vertexid_;
    std::unordered_map<graph::EdgeId,
                       std::variant<domain::StopEdge, domain::BusEdge>>
        edgeid_to_edge_;

    void BuildRouter(const transport_catalogue::TransportCatalogue& catalogue);
    void SetGraph(const transport_catalogue::TransportCatalogue& catalogue);
    void SetStopVertices(
        const std::unordered_map<std::string_view, domain::Stop*>&
            stopname_to_stop_);
    void AddEdgeToStop();
    void AddEdgeToBus(const transport_catalogue::TransportCatalogue& catalogue);
    double CalcWeight(size_t distance);
};

} // namespace router