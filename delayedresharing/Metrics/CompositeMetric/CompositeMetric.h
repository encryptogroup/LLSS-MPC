#pragma once

#include "Metrics/Metric.h"

namespace delayedresharing{
    class CompositeMetric : public Metric {
        public:
            std::vector<delayedresharing::Metric*> metrics;
            std::vector<float> coefs;
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
    };
}