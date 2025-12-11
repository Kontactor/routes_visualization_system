// geo.cpp

#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

bool operator==(const Coordinates& lhs, const Coordinates& rhs)
{
    return lhs.lat == rhs.lat && lhs.lng == rhs.lng;
}

bool operator!=(const Coordinates& lhs, const Coordinates& rhs)
{
    return !(lhs == rhs);
}

double ComputeDistance(Coordinates from, Coordinates to)
{
    using namespace std;
    const double dr = M_PI / 180.0;
    static const int earth_radius = 6371000;
    
    return acos(sin(from.lat * dr) * sin(to.lat * dr) +
                cos(from.lat * dr) * cos(to.lat * dr) *
                cos(abs(from.lng - to.lng) * dr)) *
           earth_radius;
}

} // namespace geo