#pragma once
#include "GeneralizedTerms/GeneralizedTermNetwork.h"
#include "Metrics/Metric.h"

namespace delayedresharing {

    class OptimizationPass {
        public:
            // name by which we refer to this optimization pass
            std::string name;

            // metric by which an optimization pass should work
            delayedresharing::Metric *metric;
            // Note: Some optimizations only have some of these methods on purpose

            // Applies optimization to fresh copy of GTN
            
            virtual delayedresharing::GeneralizedTermNetwork* applyFresh( delayedresharing::GeneralizedTermNetwork* termNetwork = nullptr)
            {
                assert(false);
                return nullptr;
            }

            // Applies optimization to this GTN

            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork = nullptr)
            {
                assert(false);
                return;
            }

            // Applies optimization to only this GT

            virtual void apply( delayedresharing::GeneralizedTerm* generalizedTerm = nullptr)
            {
                assert(false);
                return;
            }
    };
}