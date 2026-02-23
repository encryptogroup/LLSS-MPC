#pragma once
#include "OptimizationPass.h"
#include "ShareAssignment/ShareAssignment.h"
#include <vector>
#include <algorithm>

namespace delayedresharing {


class Ensemble : public OptimizationPass {
public:
    std::vector<OptimizationPass*> methods;
    ShareAssignment* testingAssignment;
    // Applies optimization to this GTN

    virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork = nullptr);
};
} 