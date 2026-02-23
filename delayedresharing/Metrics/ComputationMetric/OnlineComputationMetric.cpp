
#include "Metrics/ComputationMetric/ComputationMetric.h"

float delayedresharing::OnlineComputationMetric::compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
{
    int cost = 0;


    // Cost of operations and conversions
    for(int i = 0;i<termNetwork->generalizedTerms.size();i++)
    {
        cost += this->compute(termNetwork->generalizedTerms[i]);
    }


    return cost;
}



// Compute only the computation effort for a generalized Term
float delayedresharing::OnlineComputationMetric::compute( delayedresharing::GeneralizedTerm* generalizedTerm)
{
    int sum = 0;
    for(auto node : generalizedTerm->nodes)
    {
        // Computation cost of every operation performed in every wanted sharing type
        if(typeid(*node) == typeid( delayedresharing::Operation))
        {
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) node);
            // Iterate over all output sharings on which this operation is supposed to be performed
            for(std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> operationSharing : operation->operationSharings)
            {
                sum += currentCostModel->OnlineComputationOperation(operation,operationSharing).cost;
            }
        }

        // cost of every conversion
        for(auto conversion : node->conversions)
        {
            sum += currentCostModel->OnlineComputationConversion(node,conversion).cost;
        }
    }
    return sum;
}

