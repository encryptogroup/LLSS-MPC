// This is the share assignment the default way astra or aby2.0 would do it, i.e. reshare immediately after multiplication

#include "Optimizations/ShareAssignment/ShareAssignment.h"


void delayedresharing::AstraShareAssignment::apply( delayedresharing::GeneralizedTermNetwork* network)
{
    clearSharing(network);


    // inputs are allways blinded shares
    
    for( delayedresharing::Symbol* inputVariable : network->inputs->nodes)
    {
        inputVariable->sharings.insert(delayedresharing::SharingMode::BLINDED);
    }

    for(auto genTerm : network->generalizedTerms)
    {
        for(auto node : genTerm->nodes)
        {
            if(delayedresharing::Operation* operation = dynamic_cast<delayedresharing::Operation*>(node))
            {
                if(operation->operationType == delayedresharing::OperationType::MULT)
                {
                    if(operation->findInputOperations().size() >= 2)
                    {
                        operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::ADDITIVE});
                        operation->conversions.insert({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::BLINDED});
                    }else{
                        operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
                    }
                }else 
                if(operation->operationType == delayedresharing::OperationType::ADD)
                {
                        // All inputs must allready have had blinded sharing
                        operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
                }else
                if(operation->operationType == delayedresharing::OperationType::INPUT)
                { 
                        operation->operationSharings.insert({delayedresharing::SharingMode::PLAIN,delayedresharing::SharingMode::BLINDED});
                }else
                if(operation->operationType == delayedresharing::OperationType::OUTPUT)
                { 
                        operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::PLAIN});
                }else
                {
                    std::cout << "Could not share assign: " << " " << operation->operationType << " for " << node->render() << "\n";
                    assert(false);
                }
            }
            else
            {
                //std::cout << "No share assignment for node of type: " <<typeid(*node).name() << "\n";
            }
        }
    }
    network->assignSharings();

}
