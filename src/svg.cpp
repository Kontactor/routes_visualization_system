// svg.cpp

#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const
{
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap)
{
    switch (cap) {
    case StrokeLineCap::BUTT:
        return out << "butt"sv;
    case StrokeLineCap::ROUND:
        return out << "round"sv;
    case StrokeLineCap::SQUARE:
        return out << "square"sv;
    default:
        throw std::invalid_argument("Unknown StrokeLineCap value"s);
    }
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& join)
{
    switch (join) {
    case StrokeLineJoin::ARCS:
        return os << "arcs"sv;
    case StrokeLineJoin::BEVEL:
        return os << "bevel"sv;
    case StrokeLineJoin::MITER:
        return os << "miter"sv;
    case StrokeLineJoin::MITER_CLIP:
        return os << "miter-clip"sv;
    case StrokeLineJoin::ROUND:
        return os << "round"sv;
    default:
        throw std::invalid_argument("Unknown StrokeLineJoin value"s);
    }
}

struct ColorPrint {
public:
    std::ostream& out;
    void operator()(std::monostate) const
    {
        out << "none";
    }
    void operator()(std::string color) const
    {
        out << color;
    }
    void operator()(svg::Rgb color) const
    {
        out << "rgb(" << color.red << "," << color.green << ","
            << color.blue << ")";
    }
    void operator()(svg::Rgba color) const
    {
        out << "rgba(" << color.red << "," << color.green << ","
            << color.blue << "," << color.opacity << ")";
    }
};

std::ostream& operator<<(std::ostream& out, const Color& color)
{
    visit(ColorPrint{out}, color);
    return out;
}

Circle& Circle::SetCenter(Point center)
{
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)
{
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv
        << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point)
{
    polyline_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool it_first = true;
    for (Point point : polyline_) {
        if (!it_first) {
            out << ' ';
        }
        out << point.x << ',' << point.y;
        it_first = false;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos)
{
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size)
{
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data)
{
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y;
    out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
        << "\" font-size=\""sv;
    out << size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv << ReplaceSpecialChars(data_) << "</text>"sv;
}

std::string Text::ReplaceSpecialChars(const std::string& data) const
{
    std::string result;

    for (char c : data) {
        switch (c) {
        case '\"':
            result += "&quot;";
            break;
        case '\'':
            result += "&apos;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '&':
            result += "&amp;";
            break;
        default:
            result.push_back(c);
            break;
        }
    }

    return result;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj)
{
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for (const auto& object : objects_) {
        object->Render({out, 2, 2});
    }
    out << "</svg>"sv;
}

} // namespace svg