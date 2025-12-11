// map_renderer.h

#pragma once

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace map_renderer {

struct RenderSettings {
    double width;
    double height;

    double padding;

    double line_width;
    double stop_radius;

    int bus_label_font_size;
    std::array<double, 2> bus_label_offset;

    int stop_label_font_size;
    std::array<double, 2> stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;

class SphereProjector {
    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding)
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] =
            std::minmax_element(points_begin, points_end,
                                [](auto lhs, auto rhs) {
                                    return lhs.lng < rhs.lng;
                                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] =
            std::minmax_element(points_begin, points_end,
                                [](auto lhs, auto rhs) {
                                    return lhs.lat < rhs.lat;
                                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const
    {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    explicit MapRenderer(RenderSettings render_settings);

    const RenderSettings& GetRenderSettings() const;

    svg::Polyline RenderRouteLine(std::vector<svg::Point> coordinates,
                                  size_t route_counter) const;
    svg::Text RenderRouteNameSubstrate(std::string_view name,
                                       svg::Point coordinates) const;
    svg::Text RenderRouteName(std::string_view name, svg::Point coordinates,
                              size_t route_counter) const;
    svg::Circle RenderStopCircle(svg::Point coordinates) const;
    svg::Text RenderStopNameSubstrate(std::string_view name,
                                      svg::Point coordinates) const;
    svg::Text RenderStopName(std::string_view name,
                             svg::Point coordinates) const;

private:
    RenderSettings render_settings_;
};

} // namespace map_renderer