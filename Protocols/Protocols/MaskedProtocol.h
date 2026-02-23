#pragma once
#include "GeneralizedTerms/GeneralizedTermNetwork.h"
#include "GeneralizedTerms/Ring.h"

#include "Protocols/Protocol.h"
#include <algorithm>

namespace delayedresharing {


class MaskedProtocol : public Protocol {


    public:
    

        MaskedProtocol(std::string circuitFile) : Protocol(circuitFile)
        {

        }
        
        std::map<size_t, std::vector<RingElement>> gammaShares; 

        std::map<delayedresharing::Symbol*,std::vector<RingElement>> lambdaShares;

        // m value, this is the same for both parties

        std::map<delayedresharing::Symbol*,RingElement> blindedShares;

        // arithmetic shares, arithmeticShares[op][i] = <op>_i

        std::map<delayedresharing::Symbol*,std::vector<RingElement>> arithmeticShares;


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

       

        // this determines the actual value and puts it into the vector for fresh Lambdas, this is protocol dependent
        virtual void computeFreshLambdaShare(delayedresharing::Operation* operation)
        {
            assert(false);
        }


    // TODO: right now for every addition we create lambdas even if it is not needed always, this might be simplified
    virtual void computeLambdaShare(delayedresharing::Operation* operation)
    {
        if(!lambdaShares.count(operation))
        {
        bool createFreshLambda = false;
        // check if conversion from arithmetic to blinded exists if so create fresh lambda
        if(operation->hasConversionToMode(delayedresharing::SharingMode::BLINDED))
        {
            // todo maybe should check that inputs are not blinded
            createFreshLambda = true;
        }else
        {
            switch(operation->operationType)
            {
                case delayedresharing::OperationType::INPUT:
                {
                    createFreshLambda = true;
                    break;
                }
                case delayedresharing::OperationType::ADD:
                {
                    RingElement accA = 0;
                    RingElement accB = 0;
                    // sum lambdas of variadic inputs (i.e. only of operations)
                    for(delayedresharing::Symbol* inputOperation : operation->findInputOperations())
                    {
                        delayedresharing::Operation* inputOp = (delayedresharing::Operation*) inputOperation;
                        accA = RingAdd(accA,lambdaShares[inputOp][0],globalRingSize);
                        accB = RingAdd(accB,lambdaShares[inputOp][1],globalRingSize);
                    }
                    lambdaShares[operation] = {accA,accB};
                    break;
                }
                case delayedresharing::OperationType::MULT:
                {
                    // this must be mult by constant
                    const auto& inputs = operation->findInputOperations();
                    if(inputs.size() == 0)
                    {
                        // must be mult of constant and value
                        lambdaShares[operation] = {0,0};
                        break;
                    }
                    assert(inputs.size() == 1);
                    RingElement accA = 1;
                    RingElement accB = 1;
                    for(const auto& input : operation->inputs)
                    {
                        if(delayedresharing::Constant* c_input = dynamic_cast<delayedresharing::Constant*>(input))
                        {
                            accA = RingMul(accA,(RingElement)c_input->value,globalRingSize);
                            accB = RingMul(accB,(RingElement)c_input->value,globalRingSize);
                        }
                    }
                    accA = RingMul(accA,lambdaShares[inputs[0]][0],globalRingSize);
                    accB = RingMul(accB,lambdaShares[inputs[0]][1],globalRingSize);

                    lambdaShares[operation] = {accA,accB};
                }
            }
        }

        if(createFreshLambda)
        {
            // compute lambda share freshly
            computeFreshLambdaShare(operation);
        }
        }
    }

    virtual void computeGammaShare(delayedresharing::Operation* operation)
    {
        assert(false);
    }

    virtual void computeGammaShares()
    {

        for(int i = 0;i<operations.size();i++)
        {
            if(operations[i]->operationType == delayedresharing::OperationType::MULT)
            {
                computeGammaShare(operations[i]);
            }
        }
    }

    virtual void setup()
    {
        for(int i = 0;i<operations.size();i++)
        {
            if(operations[i]->hasSharing(delayedresharing::SharingMode::BLINDED))
                computeLambdaShare(operations[i]);
        }
        computeGammaShares();
        
    }


    virtual void run()
    {
        assert(false);
    }


    void reportLambdaShares()
    {
        std::cout << "Lambdas:" << "\n";
        for(auto operation : operations)
        {
            if(lambdaShares.count(operation))
                std::cout << operation->toSymbol() << ": (" << std::to_string(lambdaShares[operation][0]) << "+" << std::to_string(lambdaShares[operation][1]) << ") = " << std::to_string(RingAdd<RingElement>(lambdaShares[operation][0],lambdaShares[operation][1],globalRingSize)) << "\n";
        }
    }

    void reportGammaShares()
    {
        std::cout << "Gammas:" << "\n";
        for (const auto& pair : gammaShares) {
            std::cout << pair.first << ": (" << std::to_string(pair.second[0]) << "+" << std::to_string(pair.second[1]) << ") = " << std::to_string(RingAdd<RingElement>(pair.second[0],pair.second[1],globalRingSize)) << "\n";
        }
    }

    void reportShares()
    {
        int i = 0;
        for(const auto& operation : operations)
        {
            std::cout << i << ": arith:" << std::to_string(arithmeticShares[operation][0]) << "+" <<std::to_string(arithmeticShares[operation][1]) << " m:" << std::to_string(blindedShares[operation]) << "\n";
            i++;
        }

    }

    // TODO:: doppel inputs zb *(a,a,b) müssen set {a,a} enthalten
    std::vector<std::vector<delayedresharing::Operation*>> InputSets(delayedresharing::Operation* operation)
    {

        std::vector<delayedresharing::Operation*> inputs = operation->findInputOperations();
        // 1. get every subset of inputs with at least 2 elements
        std::vector<std::vector<delayedresharing::Operation*>> inputSets = {{}};
        for(const auto& input : inputs)
        {
            int startSize = inputSets.size();
            for(int i = 0;i<startSize;i++)
            {
                std::vector<delayedresharing::Operation*> modified(inputSets[i]);
                modified.push_back(input);
                inputSets.push_back(modified);
            }
        }

        std::vector<std::vector<delayedresharing::Operation*>> modifiedInputSets;
        for(const auto& inputSet : inputSets)
        {
            if(inputSet.size() > 1)
            {
                modifiedInputSets.push_back(inputSet);
            }
        }

        return modifiedInputSets;
    }
};

}