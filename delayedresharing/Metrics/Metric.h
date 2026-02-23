#pragma once
#include "GeneralizedTerms/GeneralizedTermNetwork.h"
#include "Metrics/CostModel.h"
#include <algorithm>

// Metrics map a Generalized Term Network with some Sharing assignment on each Generalized Term to some Cost according to the selected Cost Model

namespace delayedresharing {
    class Metric {
        public:
            CostModel* currentCostModel;
            // Note: Every metric needs to be stateless in its computation
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
            {
                assert(false);
                return 0;
            }
            virtual float compute( delayedresharing::GeneralizedTerm* term)
            {
                assert(false);
                return 0;
            }
    };
}