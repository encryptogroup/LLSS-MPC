#pragma once
#include "GeneralizedTerms/GeneralizedTermNetwork.h"
#include "GeneralizedTerms/Ring.h"
#include "Protocols/Domain.h"
#include "Protocols/ProtocolDefinitions.h"
#include <algorithm>

namespace delayedresharing {

struct ReceiveOrder {
    RingElement* store;
};

struct SendOrder {
    RingElement value;
};

class Protocol {


    public:
    
        RingElement globalRingWidth = 1;
        RingElement globalRingSize = 2;
        int batchSize = 1024;

        delayedresharing::GeneralizedTermNetwork* circuitToExecute;

        std::vector<delayedresharing::Operation*> operations;

        std::map<delayedresharing::Operation*,int> inputOwnerMap;

        std::map<delayedresharing::Operation*,int> outputOwnerMap;

        std::map<delayedresharing::Operation*,RingElement> inputs;
        
        std::vector<SendOrder> sendOrders;
        
        std::vector<ReceiveOrder> recvOrders;

        double runtime = 0;


        int lastCommunication = 0;

        std::vector<coproto::Socket> sockets;

        void reportCommunication(std::string step)
        {
            std::cout << "=== Networking of Step: " << step << "===" << "\n";

            int newCommunication = 0;
            for(auto socket : sockets)
            {
                newCommunication += socket.bytesSent() + socket.bytesReceived();
            }

            std::cout <<  (newCommunication-lastCommunication) << " Bytes" << "\n";
            std::cout << "=============================" << "\n";
            lastCommunication = newCommunication;
        }

        // care only about variadic inputs
        size_t hash_array(const std::vector<delayedresharing::Operation*> arr) 
        {
            size_t hash = 0;
            std::vector<delayedresharing::Symbol*> sortedArray;
            
            // Todo; sorting should only happen in commutative case
            for(auto v : arr)
            {
                int i = 0;
                while( (i<sortedArray.size()) && (reinterpret_cast<uintptr_t>(sortedArray[i]) > reinterpret_cast<uintptr_t>(v)))
                {
                    i++;
                }
                sortedArray.insert(sortedArray.begin() + i, v); 
            }

            for (delayedresharing::Symbol* i : sortedArray) {
                hash = 31*hash + reinterpret_cast<uintptr_t>(i);
            }
            return hash;
        }

        Protocol(std::string circuitFile) {
            circuitToExecute = delayedresharing::GeneralizedTermNetwork::fromFile(circuitFile);
            operations = circuitToExecute->OperationsInTopologicalOrder();
            globalRingWidth = std::get<0>(operations[0]->signature).ringwidth;
            globalRingSize = 1 << globalRingWidth;
            std::cout << "Ring Size: " << std::to_string(globalRingSize) << "\n";
            std::cout << "Ring Width: " << std::to_string(globalRingWidth) << "\n";
        }

    virtual void setup()
    {
        
    }


    virtual void run()
    {
        assert(false);
    }


    virtual void launch()
    {

    }

    void reportShares()
    {

    }

};

}