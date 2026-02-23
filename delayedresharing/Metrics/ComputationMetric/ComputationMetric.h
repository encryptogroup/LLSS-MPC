#pragma once
#include "Metrics/Metric.h"


namespace delayedresharing {
    class OnlineComputationMetric : public Metric {
        public:
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
    };
    class OfflineComputationMetric : public Metric {
        public:
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
    };
    class ComputationMetric : public Metric {
        public:
            OfflineComputationMetric* offlineMetric;
            OnlineComputationMetric* onlineMetric;
            ComputationMetric(CostModel* costModel){
                offlineMetric = new OfflineComputationMetric();
                offlineMetric->currentCostModel = costModel;
                onlineMetric = new OnlineComputationMetric();
                onlineMetric->currentCostModel = costModel;
            }

            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
            {
                return offlineMetric->compute(termNetwork)+onlineMetric->compute(termNetwork);
            }
            virtual float compute( delayedresharing::GeneralizedTerm* term)
            {
                return offlineMetric->compute(term)+onlineMetric->compute(term);
            }
    };
}