
#include "Metrics/CostModel.h"


bool delayedresharing::CostPoint::operator<(const CostPoint& other) const {
    return (layer < other.layer) || cost < other.cost;
}