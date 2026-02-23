#pragma once
#include "Metrics/Metric.h"


namespace delayedresharing {
    class OnlineCommunicationMetric : public Metric {
        public:
            OnlineCommunicationMetric(CostModel* costModel){
                this->currentCostModel = costModel;
            }
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
    };
    class OfflineCommunicationMetric : public Metric {
        public:
            OfflineCommunicationMetric(CostModel* costModel){
                this->currentCostModel = costModel;
            }
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
    };
    class CommunicationMetric : public Metric {
        public:
            OfflineCommunicationMetric* offlineMetric;
            OnlineCommunicationMetric* onlineMetric;
            CommunicationMetric(CostModel* costModel){
                this->currentCostModel = costModel;
                offlineMetric = new OfflineCommunicationMetric(costModel);
                onlineMetric = new OnlineCommunicationMetric(costModel);
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