#include "Delayedresharing.h"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Hello World!" << "\n";

    
    std::string inputFile(argv[1]);
    std::string outputFileBaseline(argv[2]);
    std::string outputFileOptimized(argv[3]);
    std::string costModelString(argv[4]);

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

    delayedresharing::CostModel* costModel = new delayedresharing::AnalyticalThreePCCostModel();



    delayedresharing::CommunicationMetric* metric = new delayedresharing::CommunicationMetric(costModel);

    delayedresharing::Partition* partitioningPass = new delayedresharing::Partition();
    partitioningPass->metric = metric;
    partitioningPass->k = 1;

    delayedresharing::GeneralizedTermNetwork* networkFresh = partitioningPass->applyFresh(inputTermNetwork);

    if(costModelString == "offline")
    {
        std::cout << "Offline\n";
        delayedresharing::OfflineBoundaryShareAssignment* offlineBoundaryShareAssignment = new delayedresharing::OfflineBoundaryShareAssignment();
        offlineBoundaryShareAssignment->apply(inputTermNetwork);
        exit(0);
    }

    
    delayedresharing::AstraShareAssignment* astraShareAssignment = new delayedresharing::AstraShareAssignment();
    astraShareAssignment->apply(networkFresh);
    
    networkFresh->toFile(outputFileBaseline);
    std::cout << "result: " << metric->compute(networkFresh) << "\n";
    

    
    delayedresharing::BoundaryShareAssignment* boundaryShareAssignment = new delayedresharing::BoundaryShareAssignment();
    boundaryShareAssignment->name = "Boundary Share Assignment";


    boundaryShareAssignment->assignInputOutput = true;
        if(costModelString == "3shamir")
        {

            boundaryShareAssignment->costModel = delayedresharing::BoundaryShareAssignment::Model::SHAMIR;
            boundaryShareAssignment->numParties = 3;
            boundaryShareAssignment->threshold = 2;
            std::cout << "Shamir 3 Party\n";
        }
        if(costModelString == "10shamir")
        {

            std::cout << "Shamir 10 Party\n";
            boundaryShareAssignment->numParties = 10;
            boundaryShareAssignment->threshold = 6;
            boundaryShareAssignment->costModel = delayedresharing::BoundaryShareAssignment::Model::SHAMIR;
        }
        if(costModelString == "replicated")
        {
            std::cout << "Replicated\n";
            boundaryShareAssignment->costModel = delayedresharing::BoundaryShareAssignment::Model::REPLICATED;
        }
        if(costModelString == "masked")
        {
            std::cout << "Masked\n";
            boundaryShareAssignment->costModel = delayedresharing::BoundaryShareAssignment::Model::MASKED;
        }
        if(costModelString == "weak")
        {
            std::cout << "Weak\n";
            boundaryShareAssignment->costModel = delayedresharing::BoundaryShareAssignment::Model::REPLICATED;
            boundaryShareAssignment->assignInputOutput = false;
        }


    
    boundaryShareAssignment->apply(networkFresh);


    std::cout  << "result: " << metric->compute(networkFresh);
    
    networkFresh->toFile(outputFileOptimized);

}