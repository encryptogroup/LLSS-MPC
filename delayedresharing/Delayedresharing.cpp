#include "Delayedresharing.h"

void delayedresharing::Delayedresharing::run()
{
    std::cout << "\nStarting Optimization for " << inputFile << "\n";

    delayedresharing::GeneralizedTermNetwork* inputTermNetwork;
    if(inputFile.substr(inputFile.size() - 4) == ".txt")
    {
        std::cout << "delayedresharing file format found." << "\n";
        inputTermNetwork = delayedresharing::GeneralizedTermNetwork::fromFile(inputFile);
    }else 
    if(inputFile.substr(inputFile.size() - 8) == ".bristol")
    {
        std::cout << "Bristol file format found." << "\n";
        inputTermNetwork = delayedresharing::GeneralizedTermNetwork::fromBristol(inputFile);
    }

    #ifdef DEBUG
        inputTermNetwork->toDot("dotFiles/inputnetwork.dot");
    #endif

    // this is stupid and should instead be set per operation

    delayedresharing::SizeMetric* smetric = new delayedresharing::SizeMetric(costModel);
    int inputOperationCount = smetric->compute(inputTermNetwork);
    std::cout << "Input Network has " << inputOperationCount << " Operations" << "\n";

    
    // Compute starting score
    shareAssignments[initialAssignment]->apply(inputTermNetwork);
    float currentBest = compositeMetric->compute(inputTermNetwork);
    delayedresharing::GeneralizedTermNetwork* currentBestNetwork = inputTermNetwork;
    ShareAssignment* bestAssignment = shareAssignments[initialAssignment];

    std::cout << "\nInitial Score with " << shareAssignments[initialAssignment]->name << " Sharing:\n" << currentBest << "\n\n";
    
    inputTermNetwork->toFile(outputFile);
    initial_value = currentBest;

    delayedresharing::Partition* partitioningPass = new delayedresharing::Partition();
    partitioningPass->metric = compositeMetric;
    GeneralizedTermNetwork* prededuplicateNetwork = currentBestNetwork;
    
    if(initial_deduplicate)
    {
        // Partition into single GTN and deduplicate
        partitioningPass->k = 1;
        prededuplicateNetwork = partitioningPass->applyFresh(currentBestNetwork);
        prededuplicateNetwork->generalizedTerms[1]->deduplicate();
    }

    // do actual partitioning
    int number_of_generalizedTerms = std::ceil(static_cast<double>(inputOperationCount) / generalizedTermSize);
    partitioningPass->k = std::max(number_of_generalizedTerms,1);
    std::cout << "Set size of Generalized Terms to ~" << generalizedTermSize << " Symbols\n";
    GeneralizedTermNetwork* partitionedNetwork = partitioningPass->applyFresh(prededuplicateNetwork);


    GeneralizedTermNetwork* currentNetwork = partitionedNetwork;
    logicEnsemble->testingAssignment = shareAssignments[testingAssignment]; // todo: this should be optimal cut assignment
    logicEnsemble->metric = compositeMetric;

    int lastBestIteration = currentBest;
    std::cout << "Iterations: " << iterations << "\n";
    for(int i = 0;(i<iterations) || (currentBest < lastBestIteration);i++)
    {
        std::cout << "\n\n Optimizer Iteration " << i << "\n";
        lastBestIteration = currentBest;
        // apply logic ensemble to whole network
        
        if(logicEnsemble->methods.size() > 0)
        {
            logicEnsemble->apply(currentNetwork);
        }
        // apply share ensemble to whole network
        for(auto shareAssignment : shareEnsemble)
        {
            // score per share assignment
            shareAssignment->apply(currentNetwork);
            // Evaluate Composite Metric on new Network
            float newMetricResult = compositeMetric->compute(currentNetwork);
            std::cout << shareAssignment->name << " Score " << newMetricResult << "\n";
            if(newMetricResult < currentBest)
            {
                currentBestNetwork = currentNetwork;
                bestAssignment = shareAssignment;
                currentBest = newMetricResult;
            }
        }
        bestAssignment->apply(currentBestNetwork);
        currentBest = compositeMetric->compute(currentBestNetwork);
        currentBestNetwork->toFile(outputFile);
    #ifdef DEBUG
    for(int i = 0;i<currentBestNetwork->generalizedTerms.size();i++)
    {
        currentBestNetwork->generalizedTerms[i]->toDot("dotFiles/"+std::to_string(i)+"_opt.dot");
    }
    #endif
    }

    // write optimized description to file
    currentBestNetwork->toFile(outputFile);
    final_value = currentBest;
    std::cout << "Final Score:" << final_value << "\n";

    delete smetric;
    delete partitioningPass;
    delete currentBestNetwork;
}