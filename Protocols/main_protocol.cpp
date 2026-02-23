#include "main_protocol.h"


double total_runtime = 0;

void runProtocol(int protocol, short partyId, std::string circuitFile, std::string pI, std::string pII, int width,int batchSize)
{

    delayedresharing::Protocol* party;


    switch(protocol)
    {
        case 0:
        {
            // astra
            if(partyId == 2)
            {
                // if we are helper the argument must point to party 0
                party = new delayedresharing::AstraHelper(circuitFile,pI);
            }else
            {
                // if we are online party the arguments must point to other party and helper
                party = new delayedresharing::AstraParty(partyId,circuitFile,pI,pII);
            }
            break;
        }
        case 1:
        {
            // aby
            //party = new delayedresharing::AbyParty(partyId,inputCircuit);


            break;
        }
        case 2:
        {
            if(width == 1)
            {
                party = new delayedresharing::RSSParty<delayedresharing::BooleanValue>(partyId,circuitFile,pI,pII);
            }else{
                party = new delayedresharing::RSSParty<delayedresharing::RingValue>(partyId,circuitFile,pI,pII);
            }
        }

    }
    party->batchSize = batchSize;
    for(const auto & node : party->circuitToExecute->inputs->nodes)
    {
        if(delayedresharing::Operation* inputOp = dynamic_cast<delayedresharing::Operation*>(node))
        {
            party->inputOwnerMap[inputOp] = 0;
            party->inputs[inputOp] = 0;
        }
    }

      for(const auto & node : party->circuitToExecute->outputs->nodes)
    {
        if(delayedresharing::Operation* outputOp = dynamic_cast<delayedresharing::Operation*>(node))
        {
            party->outputOwnerMap[outputOp] = 0;
        }
    }

    party->launch();
    total_runtime += party->runtime;
    return;
}

int main(int argc, char* argv[])
{
    std::cout << "Executing protocol" <<"\n";
    std::string circuitFile(argv[1]);
    std::cout << "Given circuit: " << circuitFile << "\n";

    /* 
    0 astra 
    1 aby 
    2 rss
    */
    int protocol = atoi(argv[2]);
    // 0, 1 regular parties 2 helper
    short partyId = atoi(argv[3]);

    std::string pI(argv[4]);
    std::string pII(argv[5]);
    int num_runs = 5;
    int batchSize = 1000;
    int width = atoi(argv[6]);
    num_runs = atoi(argv[7]);
    std::cout << "Batch Size: " << batchSize << "\n";
    for(int i = 0;i<num_runs;i++)
    {
        runProtocol(protocol,partyId,circuitFile,pI,pII,width,batchSize);
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    std::cout << "Party " << partyId << " Average runtime: " << (total_runtime / num_runs) << " ms" << std::endl;
}

