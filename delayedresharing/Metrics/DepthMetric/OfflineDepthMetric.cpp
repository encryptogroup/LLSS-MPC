
#include "Metrics/DepthMetric/DepthMetric.h"



// TODO, this should be redefineable via per Term metric, as depth is computed on symbols

float delayedresharing::OfflineDepthMetric::compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
{

    std::map<delayedresharing::Symbol*,int> depthMap;
    std::vector<delayedresharing::CostPoint> costPoints;
    // collect costpoints for every operation and every conversion of entire termNetwork
    for(auto generalizedTerm : termNetwork->generalizedTerms)
    {
        for(delayedresharing::Symbol* s : generalizedTerm->nodes)
        {
            
            if(typeid(*s) == typeid(delayedresharing::Operation))
            {
                delayedresharing::Operation* operation = ( delayedresharing::Operation*) s;
                for(auto operationSharing : operation->operationSharings)
                {
                    delayedresharing::CostPoint c = currentCostModel->OfflineDepthOperation(operation,operationSharing);
                    if(!c.layerAssigned)
                    {
                        switch(currentCostModel->offlineDepthMode)
                        {
                            case delayedresharing::DepthMode::SYMBOL:
                                c.layer = 2*operation->SymbolDepth(&depthMap);
                                break;
                            case delayedresharing::DepthMode::MULTIPLICATIVE:
                                c.layer = 2*operation->MultiplicativeDepth(&depthMap);
                                break;
                        }
                    }
                    c.relatedSymbol = operation;
                    costPoints.push_back(c);
                }
                for(auto conversion : operation->conversions)
                {
                    delayedresharing::CostPoint c = currentCostModel->OfflineDepthConversion(operation,conversion);
                    if(!c.layerAssigned)
                    {
                        switch(currentCostModel->offlineDepthMode)
                        {
                            case delayedresharing::DepthMode::SYMBOL:
                                c.layer = 2*operation->SymbolDepth(&depthMap) + 1;
                                break;
                            case delayedresharing::DepthMode::MULTIPLICATIVE:
                                c.layer = 2*operation->MultiplicativeDepth(&depthMap) + 1;
                                break;
                        }
                    }
                    c.relatedSymbol = operation;
                    costPoints.push_back(c);
                }
            }
        }
    }


    return foldDepth(&costPoints);
}


// Compute only the depth contributed by a generalized term
float delayedresharing::OfflineDepthMetric::compute( delayedresharing::GeneralizedTerm* generalizedTerm)
{
    std::map<delayedresharing::Symbol*,int> depthMap;
    std::vector<delayedresharing::CostPoint> costPoints;
    // collect costpoints for every operation and every conversion of this generalizedTerm: the generalizedTerm is treated as if it was a standalone GTN: the layer numbers in the costs may be larger as they are computed from the entire GTN, but they remain meaningful as they are online used affinely
        for(delayedresharing::Symbol* s : generalizedTerm->nodes)
        {
            
            if(typeid(*s) == typeid(delayedresharing::Operation))
            {
                delayedresharing::Operation* operation = ( delayedresharing::Operation*) s;
                
                for(auto operationSharing : operation->operationSharings)
                {
                    delayedresharing::CostPoint c = currentCostModel->OfflineDepthOperation(operation,operationSharing);
                    if(!c.layerAssigned)
                    {
                        switch(currentCostModel->offlineDepthMode)
                        {
                            case delayedresharing::DepthMode::SYMBOL:
                                c.layer = 2*operation->SymbolDepth(&depthMap);
                                break;
                            case delayedresharing::DepthMode::MULTIPLICATIVE:
                                c.layer = 2*operation->MultiplicativeDepth(&depthMap);
                                break;
                        }
                    }
                    c.relatedSymbol = operation;
                    costPoints.push_back(c);
                }
                for(auto conversion : operation->conversions)
                {
                    delayedresharing::CostPoint c = currentCostModel->OfflineDepthConversion(operation,conversion);
                    if(!c.layerAssigned)
                    {
                        switch(currentCostModel->offlineDepthMode)
                        {
                            case delayedresharing::DepthMode::SYMBOL:
                                c.layer = 2*operation->SymbolDepth(&depthMap)+1;
                                break;
                            case delayedresharing::DepthMode::MULTIPLICATIVE:
                                c.layer = 2*operation->MultiplicativeDepth(&depthMap)+1;
                                break;
                        }
                    }
                    c.relatedSymbol = operation;
                    costPoints.push_back(c);
                }
            }
        }
    

    return foldDepth(&costPoints);
}