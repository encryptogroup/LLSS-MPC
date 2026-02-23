#include "Metrics/CompositeMetric/CompositeMetric.h"



float delayedresharing::CompositeMetric::compute( delayedresharing::GeneralizedTermNetwork* termNetwork)
{
    float cost = 0;
    for(int i = 0;i<coefs.size();i++)
    {
        float value = coefs[i] *metrics[i]->compute(termNetwork);
        
        cost +=  value;
    }
    return cost;
}
float delayedresharing::CompositeMetric::compute( delayedresharing::GeneralizedTerm* term)
{    
    float cost = 0;
    for(int i = 0;i<coefs.size();i++)
    {
        float value = coefs[i] *metrics[i]->compute(term);
        
       cost += value;
    }
    return cost;
}