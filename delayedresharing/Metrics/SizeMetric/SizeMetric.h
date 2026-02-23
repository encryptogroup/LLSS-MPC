#pragma once
#include "Metrics/Metric.h"

// Note: we assume that all nodes of a generalized Term are in topological order
namespace delayedresharing {
    class SizeMetric : public Metric {
        public:
            SizeMetric(CostModel* costModel){
            }
            
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
            {
                int size = 0;
                for(const auto& genTerm : termNetwork->generalizedTerms)
                {
                    size += compute(genTerm);
                }

                return size;
            }

            virtual float compute( delayedresharing::GeneralizedTerm* term)
            {
                int size = 0;
                for(const auto& node : term->nodes)
                {
                    if(typeid(*node) != typeid(delayedresharing::Variable))
                    {
                        size++;
                    }
                }
                return size;
            }
    };
    
}