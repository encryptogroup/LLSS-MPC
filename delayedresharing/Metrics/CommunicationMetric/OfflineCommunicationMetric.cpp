
#include "Metrics/CommunicationMetric/CommunicationMetric.h"

float delayedresharing::OfflineCommunicationMetric::compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
{
    int cost = 0;


    // Cost of operations and conversions
    for(int i = 0;i<termNetwork->generalizedTerms.size();i++)
    {
        int value = this->compute(termNetwork->generalizedTerms[i]);
        cost += value;
        
        #ifdef DEBUG
                //std::cout << "Offline Communication Cost for Generalized Term " <<  i << " = " << value << "\n";
        #endif
    }

    return cost;
}



// Compute only the communication effort for a generalized Term
float delayedresharing::OfflineCommunicationMetric::compute( delayedresharing::GeneralizedTerm* generalizedTerm)
{
    float sum = 0.0f;
    for(auto node : generalizedTerm->nodes)
    {
        int nodeCost = 0;
        // Computation cost of every operation performed in every wanted sharing type
        if(typeid(*node) == typeid( delayedresharing::Operation))
        {
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) node);
            // Iterate over all output sharings on which this operation is supposed to be performed
            for(std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> operationSharing : operation->operationSharings)
            {
                float costPoint = currentCostModel->OfflineCommunicationOperation(operation,operationSharing).cost;
                nodeCost +=  costPoint;    
            }
        }

        // cost of every conversion
        for(auto conversion : node->conversions)
        {
            nodeCost += currentCostModel->OfflineCommunicationConversion(node,conversion).cost;
        }
        sum += nodeCost;
    }
    return sum;
}

