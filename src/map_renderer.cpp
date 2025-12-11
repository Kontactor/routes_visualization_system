// map_renderer.cpp

#include "map_renderer.h"

namespace map_renderer {

MapRenderer::MapRenderer(RenderSettings render_settings)
    : render_settings_(render_settings)
{
}

const RenderSettings& MapRenderer::GetRenderSettings() const
{
    return render_settings_;
}

svg::Polyline MapRenderer::RenderRouteLine(
    std::vector<svg::Point> coordinates, size_t route_counter) const
{
    svg::Polyline route;
    for (const auto& point : coordinates) {
        route.AddPoint(point);
    }
    route.SetFillColor("none");
    route.SetStrokeColor(
        render_settings_.color_palette[route_counter %
                                       render_settings_.color_palette.size()]);
    route.SetStrokeWidth(render_settings_.line_width);
    route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return route;
}

svg::Text MapRenderer::RenderRouteNameSubstrate(std::string_view name,
                                                svg::Point coordinates) const
{
    svg::Text route_name_substrate;

    route_name_substrate.SetPosition(coordinates);
    route_name_substrate.SetOffset(
        {render_settings_.bus_label_offset.at(0),
         render_settings_.bus_label_offset.at(1)});
    route_name_substrate.SetFontSize(
        static_cast<uint32_t>(render_settings_.bus_label_font_size));
    route_name_substrate.SetFontFamily("Verdana");
    route_name_substrate.SetFontWeight("bold");
    route_name_substrate.SetData(std::string(name));
    route_name_substrate.SetFillColor(render_settings_.underlayer_color);
    route_name_substrate.SetStrokeColor(render_settings_.underlayer_color);
    route_name_substrate.SetStrokeWidth(render_settings_.underlayer_width);
    route_name_substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    route_name_substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return route_name_substrate;
}

svg::Text MapRenderer::RenderRouteName(std::string_view name,
                                       svg::Point coordinates,
                                       size_t route_counter) const
{
    svg::Text route_name;

    route_name.SetPosition(coordinates);
    route_name.SetOffset(
        {render_settings_.bus_label_offset.at(0),
         render_settings_.bus_label_offset.at(1)});
    route_name.SetFontSize(
        static_cast<uint32_t>(render_settings_.bus_label_font_size));
    route_name.SetFontFamily("Verdana");
    route_name.SetFontWeight("bold");
    route_name.SetData(std::string(name));
    route_name.SetFillColor(
        render_settings_.color_palette[route_counter %
                                       render_settings_.color_palette.size()]);

    return route_name;
}

svg::Circle MapRenderer::RenderStopCircle(svg::Point coordinates) const
{
    svg::Circle stop_circle;

    stop_circle.SetCenter(coordinates);
    stop_circle.SetRadius(render_settings_.stop_radius);
    stop_circle.SetFillColor("white");

    return stop_circle;
}

svg::Text MapRenderer::RenderStopNameSubstrate(std::string_view name,
                                               svg::Point coordinates) const
{
    svg::Text stop_name_substrate;

    stop_name_substrate.SetPosition(coordinates);
    stop_name_substrate.SetOffset(
        {render_settings_.stop_label_offset.at(0),
         render_settings_.stop_label_offset.at(1)});
    stop_name_substrate.SetFontSize(
        static_cast<uint32_t>(render_settings_.stop_label_font_size));
    stop_name_substrate.SetFontFamily("Verdana");
    stop_name_substrate.SetData(std::string(name));
    stop_name_substrate.SetFillColor(render_settings_.underlayer_color);
    stop_name_substrate.SetStrokeColor(render_settings_.underlayer_color);
    stop_name_substrate.SetStrokeWidth(render_settings_.underlayer_width);
    stop_name_substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    stop_name_substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return stop_name_substrate;
}

svg::Text MapRenderer::RenderStopName(std::string_view name,
                                      svg::Point coordinates) const
{
    svg::Text stop_name;

    stop_name.SetPosition(coordinates);
    stop_name.SetOffset(
        {render_settings_.stop_label_offset.at(0),
         render_settings_.stop_label_offset.at(1)});
    stop_name.SetFontSize(
        static_cast<uint32_t>(render_settings_.stop_label_font_size));
    stop_name.SetFontFamily("Verdana");
    stop_name.SetData(std::string(name));
    stop_name.SetFillColor("black");

    return stop_name;
}

} // namespace map_renderer