// domain.cpp

#include "domain.h"

namespace domain {

size_t HasherPairPtr::operator()(
    const std::pair<Stop*, Stop*>& pair_ptr) const
{
    return (std::hash<Stop*>()(pair_ptr.first) * 11) +
           (std::hash<Stop*>()(pair_ptr.second) * 11 * 11);
}

} // namespace domain