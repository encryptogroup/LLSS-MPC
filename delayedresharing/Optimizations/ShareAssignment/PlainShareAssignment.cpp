#include "Optimizations/ShareAssignment/ShareAssignment.h"



void delayedresharing::PlainShareAssignment::apply( delayedresharing::GeneralizedTermNetwork* network)
{
    clearSharing(network);
    for(auto generalizedTerm : network->generalizedTerms)
    {
        for(auto node : generalizedTerm->nodes)
        {
            if(typeid(*node) == typeid(delayedresharing::Operation))
            {
                delayedresharing::Operation* operation = (delayedresharing::Operation*) node;
                operation->operationSharings.insert({delayedresharing::SharingMode::PLAIN,delayedresharing::SharingMode::PLAIN});
            }
        }
    }


    network->assignSharings();
}
