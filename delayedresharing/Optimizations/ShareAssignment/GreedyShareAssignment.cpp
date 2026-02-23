#include "Optimizations/ShareAssignment/ShareAssignment.h"

// Note this is currently not a real greedy assignment but instead fixed upon the arithmetic blinded context
// real greedy would take an initial assignment of input variables and propagate it as cheaply as possible according to cost model

void delayedresharing::GreedyShareAssignment::apply(delayedresharing::GeneralizedTerm* genTerm)
{

    clearSharing(genTerm);
    // inputs are allways blinded shares
    
    

    std::vector<delayedresharing::Operation*> operationsToProcess;
    std::map<delayedresharing::Symbol*,int> depthMap;


        for(const auto& node : genTerm->nodes)
        {
            if(typeid(*node) == typeid( delayedresharing::Operation))
            {
                operationsToProcess.push_back((delayedresharing::Operation*) node);
                node->SymbolDepth(&depthMap);
            }
        }

    std::sort(operationsToProcess.begin(), operationsToProcess.end(), [&depthMap](delayedresharing::Operation* a, delayedresharing::Operation* b) {
        if(depthMap[a] == depthMap[b])
        {
            // make ordering more unique by number of inputs: this has impact on quality of solve, as larger parts may mean larger savings
            int sizeInA = a->inputs.size();
            int sizeInB = b->inputs.size();
            if(sizeInA == sizeInB)
            {
                return(a->defUses.size() < b->defUses.size());
            }
            return (sizeInA < sizeInB);
        }
        return depthMap[a] < depthMap[b];
    });


    for(delayedresharing::Operation* operation : operationsToProcess)
    {
                if(operation->operationType == delayedresharing::OperationType::MULT)
                {
                    operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::ADDITIVE});
                    operation->sharings.insert(delayedresharing::SharingMode::ADDITIVE);
                    for( delayedresharing::Operation* inputSymbol : operation->findInputOperations())
                    {
                        if((inputSymbol != nullptr) && !inputSymbol->hasSharing( delayedresharing::SharingMode::BLINDED))
                        { 
                            inputSymbol->conversions.insert({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::BLINDED});
                            inputSymbol->sharings.insert(delayedresharing::SharingMode::BLINDED);
                        }
                    }
                }else 
                if(operation->operationType == delayedresharing::OperationType::ADD)
                {
                    // determine sharing most inputs have, add missing resharings to others, prefer blinded
                    // NOTE: if even one input is arithmetic, do arithmetic, because resharing this single input and then adding for some later use in multiplication is just as expensive as resharing after the fact
                    // this worsens if multiple inputs are arithmetic
                    bool doArithmetic = false;
                    // note: sharing of constants does not matter for this decision
                    for( delayedresharing::Operation* inputSymbol : operation->findInputOperations())
                    {
                        if(!inputSymbol->hasSharing( delayedresharing::SharingMode::BLINDED))
                        {
                            doArithmetic = true;
                            break;
                        }
                    }
                    if(doArithmetic)
                    {
                        for( delayedresharing::Operation* inputSymbol : operation->findInputOperations())
                        {
                            if((inputSymbol != nullptr) && !inputSymbol->hasSharing(delayedresharing::SharingMode::ADDITIVE))
                            {
                                inputSymbol->conversions.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::ADDITIVE});
                                inputSymbol->sharings.insert(delayedresharing::SharingMode::ADDITIVE);
                            }
                        }
                        operation->operationSharings.insert({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::ADDITIVE});
                        operation->sharings.insert(delayedresharing::SharingMode::ADDITIVE);
                    }else{
                        // All inputs must allready have had blinded sharing
                        operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
                        operation->sharings.insert(delayedresharing::SharingMode::BLINDED);
                    }
                }else
                if(operation->operationType == delayedresharing::OperationType::INPUT)
                {
                    // we only ever input share blinded
                    operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
                    operation->sharings.insert(delayedresharing::SharingMode::BLINDED);
                }else 
                if(operation->operationType == delayedresharing::OperationType::OUTPUT)
                {
                    // just reveal in sharing scheme of only input, this will allways be arithmetic, as
                    auto outputSharings = (operation->findInputOperations()[0]->sharings);
                    for(auto sharing : outputSharings)
                    {
                        operation->operationSharings.insert({sharing,sharing});
                        operation->sharings.insert(sharing);
                        break;
                    }
                }
                else{
                    std::cout << "Could not share assign: " << operation->toSymbol() << "\n";
                    assert(false);
                }
    }

    // for rewriting rules we do not consider the sharing assignment selections of our variables as our problem.
    // this is part of the circuit context: variables could have any share assignment as they are related to other symbols
    for(const auto& v : genTerm->inputVariables)
    {
        v->sharings.clear();
        v->conversions.clear();
    }
    for(const auto& v : genTerm->outputVariables)
    {
        v->sharings.clear();
        v->conversions.clear();
    }
}

void delayedresharing::GreedyShareAssignment::apply( delayedresharing::GeneralizedTermNetwork* network)
{
    clearSharing(network);
    network->assignDefUses();
    // inputs are allways blinded shares
    
    

    std::vector<delayedresharing::Operation*> operationsToProcess;
    std::map<delayedresharing::Symbol*,int> depthMap;

    for(auto genTerm : network->generalizedTerms)
    {
        for(auto node : genTerm->nodes)
        {
            if(typeid(*node) == typeid( delayedresharing::Operation))
            {
                operationsToProcess.push_back((delayedresharing::Operation*) node);
                node->SymbolDepth(&depthMap);
            }
        }
    }

    std::sort(operationsToProcess.begin(), operationsToProcess.end(), [&depthMap](delayedresharing::Operation* a, delayedresharing::Operation* b) {
        if(depthMap[a] == depthMap[b])
        {
            // make ordering more unique by number of inputs: this has impact on quality of solve, as larger parts may mean larger savings
            int sizeInA = a->inputs.size();
            int sizeInB = b->inputs.size();
            if(sizeInA == sizeInB)
            {
                return(a->defUses.size() < b->defUses.size());
            }
            return (sizeInA < sizeInB);
        }
        return depthMap[a] < depthMap[b];
    });


    for(delayedresharing::Operation* operation : operationsToProcess)
    {
                if(operation->operationType == delayedresharing::OperationType::MULT)
                {
                    operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::ADDITIVE});
                    operation->sharings.insert(delayedresharing::SharingMode::ADDITIVE);
                    for( delayedresharing::Operation* inputSymbol : operation->findInputOperations())
                    {
                        if(!inputSymbol->hasSharing( delayedresharing::SharingMode::BLINDED))
                        { 
                            inputSymbol->conversions.insert({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::BLINDED});
                            inputSymbol->sharings.insert(delayedresharing::SharingMode::BLINDED);
                        }
                    }
                }else 
                if(operation->operationType == delayedresharing::OperationType::ADD)
                {
                    // determine sharing most inputs have, add missing resharings to others, prefer blinded
                    // NOTE: if even one input is arithmetic, do arithmetic, because resharing this single input and then adding for some later use in multiplication is just as expensive as resharing after the fact
                    // this worsens if multiple inputs are arithmetic
                    bool doArithmetic = false;
                    // note: sharing of constants does not matter for this decision
                    for( delayedresharing::Operation* inputSymbol : operation->findInputOperations())
                    {
                        if(!inputSymbol->hasSharing( delayedresharing::SharingMode::BLINDED))
                        {
                            doArithmetic = true;
                            break;
                        }
                    }
                    if(doArithmetic)
                    {
                        for( delayedresharing::Operation* inputSymbol : operation->findInputOperations())
                        {
                            if(!inputSymbol->hasSharing(delayedresharing::SharingMode::ADDITIVE))
                            {
                                inputSymbol->conversions.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::ADDITIVE});
                                inputSymbol->sharings.insert(delayedresharing::SharingMode::ADDITIVE);
                            }
                        }
                        operation->operationSharings.insert({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::ADDITIVE});
                        operation->sharings.insert(delayedresharing::SharingMode::ADDITIVE);
                    }else{
                        // All inputs must allready have had blinded sharing
                        operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
                        operation->sharings.insert(delayedresharing::SharingMode::BLINDED);
                    }
                }else
                if(operation->operationType == delayedresharing::OperationType::INPUT)
                {
                    // we only ever input share blinded
                    operation->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
                    operation->sharings.insert(delayedresharing::SharingMode::BLINDED);
                }else 
                if(operation->operationType == delayedresharing::OperationType::OUTPUT)
                {
                    // just reveal in sharing scheme of only input, this will allways be arithmetic, as
                    auto outputSharings = (operation->findInputOperations()[0]->sharings);
                    for(auto sharing : outputSharings)
                    {
                        operation->operationSharings.insert({sharing,sharing});
                        operation->sharings.insert(sharing);
                        break;
                    }
                }
                else{
                    std::cout << "Could not share assign: " << operation->toSymbol() << "\n";
                    assert(false);
                }
    }

    consistencyCheck(network);

}
