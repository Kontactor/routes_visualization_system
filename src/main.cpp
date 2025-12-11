// main.cpp

#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

int main()
{
    json_reader::JsonReader reader(std::cin);

    transport_catalogue::TransportCatalogue catalog =
        reader.ReadTransportCatalogue();

    map_renderer::MapRenderer map_renderer(reader.FillRenderSettings());

    request_handler::RequestHandler handler(catalog, map_renderer);

    std::istringstream strm(reader.GenerateResponses(handler).data);

    json::Print(json::Load(strm), std::cout);

    return 0;
}