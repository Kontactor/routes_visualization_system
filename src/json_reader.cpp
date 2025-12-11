// json_reader.cpp

#include "json_reader.h"

namespace json_reader {

struct EdgeInfoGetter {
    json::Node operator()(const domain::StopEdge& edge_info)
    {
        return json::Builder{}
            .StartDict()
            .Key("type")
            .Value("Wait")
            .Key("stop_name")
            .Value(std::string(edge_info.stopptr->name))
            .Key("time")
            .Value(edge_info.time)
            .EndDict()
            .Build();
    }

    json::Node operator()(const domain::BusEdge& edge_info)
    {
        return json::Builder{}
            .StartDict()
            .Key("type")
            .Value("Bus")
            .Key("bus")
            .Value(std::string(edge_info.busptr->name))
            .Key("span_count")
            .Value(static_cast<int>(edge_info.span_count))
            .Key("time")
            .Value(edge_info.time)
            .EndDict()
            .Build();
    }
};

svg::Color NodeToSVGColor(const json::Node& node_color)
{
    if (node_color.IsArray()) {
        if (node_color.AsArray().size() == 3) {
            svg::Rgb color;
            color.red = static_cast<uint16_t>(
                node_color.AsArray()[0].AsInt());
            color.green = static_cast<uint16_t>(
                node_color.AsArray()[1].AsInt());
            color.blue = static_cast<uint16_t>(
                node_color.AsArray()[2].AsInt());
            return color;
        } else {
            svg::Rgba color;
            color.red = static_cast<uint16_t>(
                node_color.AsArray()[0].AsInt());
            color.green = static_cast<uint16_t>(
                node_color.AsArray()[1].AsInt());
            color.blue = static_cast<uint16_t>(
                node_color.AsArray()[2].AsInt());
            color.opacity = node_color.AsArray()[3].AsDouble();
            return color;
        }
    }
    return node_color.AsString();
}

transport_catalogue::TransportCatalogue JsonReader::ReadTransportCatalogue()
    const
{
    transport_catalogue::TransportCatalogue transport_catalogue;
    json::Dict dict = doc_.GetRoot().AsDict();
    json::Array requests = dict.at("base_requests").AsArray();

    for (const auto& request : requests) {
        if (request.AsDict().at("type").AsString() == "Stop") {
            std::unordered_map<std::string_view, size_t> length_to_stop;
            for (const auto& [name, length] :
                 request.AsDict().at("road_distances").AsDict()) {
                length_to_stop[name] = static_cast<size_t>(length.AsInt());
            }
            transport_catalogue.AddStop(
                request.AsDict().at("name").AsString(),
                {request.AsDict().at("latitude").AsDouble(),
                 request.AsDict().at("longitude").AsDouble()},
                length_to_stop);
        }
    }

    for (const auto& request : requests) {
        if (request.AsDict().at("type").AsString() == "Bus") {
            std::vector<std::string_view> stops;
            for (const auto& stop :
                 request.AsDict().at("stops").AsArray()) {
                stops.push_back(stop.AsString());
            }
            if (!request.AsDict().at("is_roundtrip").AsBool()) {
                for (int it = static_cast<int>(stops.size() - 2); it >= 0;
                     --it) {
                    stops.push_back(stops[static_cast<size_t>(it)]);
                }
            }
            transport_catalogue.AddRoute(
                request.AsDict().at("name").AsString(), stops,
                request.AsDict().at("is_roundtrip").AsBool());
        }
    }
    return transport_catalogue;
}

map_renderer::RenderSettings JsonReader::FillRenderSettings() const
{
    json::Dict dict = doc_.GetRoot().AsDict();
    json::Dict dict_settings = dict.at("render_settings").AsDict();
    map_renderer::RenderSettings render_settings;
    render_settings.width = dict_settings.at("width").AsDouble();
    render_settings.height = dict_settings.at("height").AsDouble();

    render_settings.padding = dict_settings.at("padding").AsDouble();

    render_settings.line_width = dict_settings.at("line_width").AsDouble();
    render_settings.stop_radius = dict_settings.at("stop_radius").AsDouble();

    render_settings.bus_label_font_size =
        dict_settings.at("bus_label_font_size").AsInt();
    render_settings.bus_label_offset[0] =
        dict_settings.at("bus_label_offset").AsArray()[0].AsDouble();
    render_settings.bus_label_offset[1] =
        dict_settings.at("bus_label_offset").AsArray()[1].AsDouble();

    render_settings.stop_label_font_size =
        dict_settings.at("stop_label_font_size").AsInt();
    render_settings.stop_label_offset[0] =
        dict_settings.at("stop_label_offset").AsArray()[0].AsDouble();
    render_settings.stop_label_offset[1] =
        dict_settings.at("stop_label_offset").AsArray()[1].AsDouble();

    render_settings.underlayer_color =
        NodeToSVGColor(dict_settings.at("underlayer_color"));
    render_settings.underlayer_width =
        dict_settings.at("underlayer_width").AsDouble();

    for (const auto& color :
         dict_settings.at("color_palette").AsArray()) {
        render_settings.color_palette.push_back(NodeToSVGColor(color));
    }
    return render_settings;
}

domain::RouterSettings JsonReader::FillRouterSettings() const
{
    json::Dict dict = doc_.GetRoot().AsDict();
    json::Dict dict_settings = dict.at("routing_settings").AsDict();
    domain::RouterSettings router_settings;

    router_settings.bus_wait_time =
        dict_settings.at("bus_wait_time").AsDouble();
    router_settings.bus_velocity =
        dict_settings.at("bus_velocity").AsDouble();

    return router_settings;
}

Response JsonReader::GenerateResponses(
    request_handler::RequestHandler& handler) const
{
    json::Dict dict = doc_.GetRoot().AsDict();
    json::Array requests = dict.at("stat_requests").AsArray();
    json::Array response_data;

    router::TransportRouter router(
        handler.GetTransportCatalogue(), FillRouterSettings(),
        handler.GetTransportCatalogue().GetAllStopsCount());

    for (const auto& request : requests) {
        if (request.AsDict().at("type").AsString() == "Bus") {
            if (handler
                    .GetRouteStat(request.AsDict().at("name").AsString())
                    .has_value()) {
                domain::RouteStats route_stats =
                    handler
                        .GetRouteStat(request.AsDict().at("name").AsString())
                        .value();

                json::Node response =
                    json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.AsDict().at("id").AsInt())
                        .Key("curvature")
                        .Value(static_cast<double>(route_stats.route_length) /
                               route_stats.geo_distance)
                        .Key("route_length")
                        .Value(static_cast<int>(route_stats.route_length))
                        .Key("stop_count")
                        .Value(static_cast<int>(route_stats.stops_count))
                        .Key("unique_stop_count")
                        .Value(static_cast<int>(route_stats.uniquestops_count))
                        .EndDict()
                        .Build();
                response_data.push_back(response.AsDict());
            } else {
                json::Node response =
                    json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.AsDict().at("id").AsInt())
                        .Key("error_message")
                        .Value("not found")
                        .EndDict()
                        .Build();
                response_data.push_back(response.AsDict());
            }
        } else if (request.AsDict().at("type").AsString() == "Stop") {
            if (handler
                    .GetStop(request.AsDict().at("name").AsString())
                    .has_value()) {
                json::Array buses;
                std::set<std::string_view> buses_on_stop =
                    handler.GetBusesByStop(
                        request.AsDict().at("name").AsString());
                for (auto& bus : buses_on_stop) {
                    buses.push_back(std::string(bus));
                }
                json::Node response =
                    json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.AsDict().at("id").AsInt())
                        .Key("buses")
                        .Value(buses)
                        .EndDict()
                        .Build();
                response_data.push_back(response.AsDict());
            } else {
                json::Node response =
                    json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.AsDict().at("id").AsInt())
                        .Key("error_message")
                        .Value("not found")
                        .EndDict()
                        .Build();
                response_data.push_back(response.AsDict());
            }
        } else if (request.AsDict().at("type").AsString() == "Map") {
            std::ostringstream outputStream;
            handler.RenderMap().Render(outputStream);
            std::string str = "\"" + outputStream.str() + "\"";

            json::Node response =
                json::Builder{}
                    .StartDict()
                    .Key("request_id")
                    .Value(request.AsDict().at("id").AsInt())
                    .Key("map")
                    .Value(outputStream.str())
                    .EndDict()
                    .Build();
            response_data.push_back(response.AsDict());
        } else if (request.AsDict().at("type").AsString() == "Route") {
            const auto& route_info =
                GetRouteInfo(request.AsDict().at("from").AsString(),
                             request.AsDict().at("to").AsString(), router,
                             handler);
            if (route_info.has_value()) {
                json::Array items;
                for (const auto& item : route_info->edges) {
                    items.emplace_back(std::visit(EdgeInfoGetter{}, item));
                }

                json::Node response =
                    json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.AsDict().at("id").AsInt())
                        .Key("total_time")
                        .Value(route_info->total_time)
                        .Key("items")
                        .Value(items)
                        .EndDict()
                        .Build();
                response_data.push_back(response.AsDict());
            } else {
                json::Node response =
                    json::Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.AsDict().at("id").AsInt())
                        .Key("error_message")
                        .Value("not found")
                        .EndDict()
                        .Build();
                response_data.push_back(response.AsDict());
            }
        }
    }
    std::ostringstream outputStream;
    json::Print(json::Document(response_data), outputStream);
    return {"success", "json", outputStream.str()};
}

std::optional<domain::RouteInfo> JsonReader::GetRouteInfo(
    std::string_view from, std::string_view to,
    router::TransportRouter& router,
    request_handler::RequestHandler& handler) const
{
    domain::Stop* begin =
        handler.GetTransportCatalogue().FindStop(from).value();
    domain::Stop* finish =
        handler.GetTransportCatalogue().FindStop(to).value();
    graph::VertexId start =
        router.GetVertexIdByStop(begin)->bus_wait_start;
    graph::VertexId end =
        router.GetVertexIdByStop(finish)->bus_wait_start;
    return router.GetRouteInfo(start, end);
}

} // namespace json_reader