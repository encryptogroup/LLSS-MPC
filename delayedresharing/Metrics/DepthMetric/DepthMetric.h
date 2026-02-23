#pragma once
#include "Metrics/Metric.h"

// Note: we assume that all nodes of a generalized Term are in topological order
namespace delayedresharing {

 

    class OnlineDepthMetric : public Metric {
        public:
           int foldDepth(std::vector<delayedresharing::CostPoint>* costPoints)
            {

            // sort vector
            std::sort(costPoints->begin(),costPoints->end());

            // accumulate across differing layers
            int depthAccumulator = 0;
            int currentLayer = 0;
            int biggestDepthOfLayer = 0;
                    for(int i = 0;i<costPoints->size();i++)
                    {
                        bool layerChanged = (*costPoints)[i].layer != currentLayer;
                        if(layerChanged)
                        {
                            currentLayer = (*costPoints)[i].layer;
                            depthAccumulator += biggestDepthOfLayer;
                            biggestDepthOfLayer = 0;
                        }
                        if(biggestDepthOfLayer < (*costPoints)[i].cost)
                        {
                            biggestDepthOfLayer = (*costPoints)[i].cost;
                        }
                    }
                    depthAccumulator += biggestDepthOfLayer;
                
                return depthAccumulator;
            }
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
            
            OnlineDepthMetric(CostModel* costModel){
                currentCostModel = costModel;
            }
    };
    class OfflineDepthMetric : public Metric {
        public:
           int foldDepth(std::vector<delayedresharing::CostPoint>* costPoints)
            {

            // sort vector
            std::sort(costPoints->begin(),costPoints->end());

            // accumulate across differing layers
            int depthAccumulator = 0;
            int currentLayer = 0;
            int biggestDepthOfLayer = 0;
                    for(int i = 0;i<costPoints->size();i++)
                    {
                        bool layerChanged = (*costPoints)[i].layer != currentLayer;
                        if(layerChanged)
                        {
                            currentLayer = (*costPoints)[i].layer;
                            depthAccumulator += biggestDepthOfLayer;
                            biggestDepthOfLayer = 0;
                        }
                        if(biggestDepthOfLayer < (*costPoints)[i].cost)
                        {
                            biggestDepthOfLayer = (*costPoints)[i].cost;
                        }
                    }
                    depthAccumulator += biggestDepthOfLayer;
                return depthAccumulator;
            }
            virtual float compute( delayedresharing::GeneralizedTermNetwork* termNetwork);
            virtual float compute( delayedresharing::GeneralizedTerm* term);
    };
    class DepthMetric : public Metric {
        public:
            OfflineDepthMetric* offlineMetric;
            OnlineDepthMetric* onlineMetric;
            DepthMetric(CostModel* costModel){
                offlineMetric = new OfflineDepthMetric();
                offlineMetric->currentCostModel = costModel;
                onlineMetric = new OnlineDepthMetric(costModel);
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