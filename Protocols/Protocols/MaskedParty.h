#pragma once


#include "Protocols/MaskedProtocol.h"



namespace delayedresharing {


// TODO: here everything that is the same for both parties, i.e. only the way setup is done, is varied
    
class MaskedParty : public MaskedProtocol {

    public:
        // channel connecting to other online party
        coproto::Socket partyChannel;

        short partyId;

        // prng shared between this party and other online party
        osuCrypto::PRNG partyPRNG;

        int bytesTransferredOffline;
        int bytesTransferredOnline;



        MaskedParty(short party, std::string circuitFile, std::string otherPartyIP) : MaskedProtocol(circuitFile) {
            this->partyId = party;
            std::cout << "Connecting to other party " << otherPartyIP << "\n";
            this->partyChannel = coproto::asioConnect(otherPartyIP+":13555", party == 0);
            std::cout << "Connected to other party" << "\n";
            
            // seed is binary of included parties 011 = only party 0 and 1 not helper
            partyPRNG = osuCrypto::PRNG(osuCrypto::toBlock(3));
            sockets.push_back((this->partyChannel));
        }



    void evaluateInput(delayedresharing::Operation* inputOp,int step)
    {
        switch(step)
        {
            case 0:
                // evaluating input operation
                RingElement m;
                if(inputOwnerMap[inputOp] == partyId)
                {
                    // this party is owner of input
                    RingElement lambda = RingAdd<RingElement>(lambdaShares[inputOp][0], lambdaShares[inputOp][1],globalRingSize);
                
                    m = RingSub<RingElement>(inputs[inputOp],lambda,globalRingSize);
                    blindedShares[inputOp] = m;
                    sendOrders.push_back({value: m});
                }else
                {
                    blindedShares[inputOp] = 0;
                    recvOrders.push_back({store: &blindedShares.at(inputOp)});
                }
            break;
        }
    }

    void evaluateMult(delayedresharing::Operation* multOp,int step)
    {
        if(multOp->operationSharings.count({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::ADDITIVE}))
        {
            evaluateMultBlindedArithmetic(multOp,step);
        }

        
        // must be mult of constant and value
        if(multOp->operationSharings.count({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED}))
        {
            RingElement acc = 1;
            for(const auto& input : multOp->inputs)
            {
                if(delayedresharing::Constant* c_input = dynamic_cast<delayedresharing::Constant*>(input))
                {
                    acc = RingMul(acc,(RingElement) c_input->value,globalRingSize);
                }
            }

            auto var_inputs = multOp->findInputOperations();
            if(var_inputs.size() > 0)
                acc = RingMul(acc,blindedShares[var_inputs[0]],globalRingSize);

            blindedShares[multOp] = acc;

        }
        if(multOp->operationSharings.count({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::ADDITIVE}))
        {
            RingElement acc = 1;
            for(const auto& input : multOp->inputs)
            {
                if(delayedresharing::Constant* c_input = dynamic_cast<delayedresharing::Constant*>(input))
                {
                    acc = RingMul(acc,(RingElement) c_input->value,globalRingSize);
                }
            }
            auto var_inputs = multOp->findInputOperations();
            if(var_inputs.size() > 0)
            acc = RingMul(acc,arithmeticShares[var_inputs[0]][partyId],globalRingSize);

            arithmeticShares[multOp] = {(partyId==0)*acc,(partyId==1)*acc};
        }
    }

    void evaluateMultBlindedArithmetic(delayedresharing::Operation* multOp,int step)
    {
        switch(step)
        {
        case 0:
            std::vector<delayedresharing::Operation*> inputs = multOp->findInputOperations();
            

            // 1. get every subset of inputs with at least 2 elements
            std::vector<std::vector<delayedresharing::Operation*>> inputSets = InputSets(multOp);

            RingElement arithShare = 0;

            // 2. add m of all inputs
            if(partyId == 0)
            {
                RingElement mInputs = 1;
                for(delayedresharing::Operation* input : inputs)
                {
                    mInputs = RingMul<RingElement>(mInputs,blindedShares[input],globalRingSize);
                    
                }
                arithShare = RingAdd<RingElement>(arithShare,mInputs,globalRingSize);
            }
            // 3. add gamma crossterms
            for(std::vector<delayedresharing::Operation*> inputSet : inputSets)
            {
                RingElement m = 1;
                std::vector<delayedresharing::Operation*> m_inputs(inputs);

                for(const auto& inputOfSet : inputSet)
                {
                    std::vector<delayedresharing::Operation*> modified;
                    
                    bool removed = false;
                    for(const auto& m_in : m_inputs)
                    {
                        bool insert = true;
                        if(!removed)
                        {
                            if(inputOfSet == m_in)
                            {
                                insert = false;
                                removed = true;
                            }
                        }
                        if(insert)
                        {
                            modified.push_back(m_in);
                        }
                    }
                    m_inputs = modified;
                }

                for(delayedresharing::Operation* input : m_inputs)
                {
                    m = RingMul<RingElement>(m,blindedShares[input],globalRingSize);
                }

                size_t index = hash_array(inputSet);
                RingElement gamma = gammaShares[index][partyId];
                RingElement g = RingMul<RingElement>(gamma,m,globalRingSize);

                arithShare = RingAdd<RingElement>(arithShare,g,globalRingSize);
            }

            // 4. add lambda crossterms = one lambda and all other m's

            for(int i = 0;i<inputs.size();i++)
            {
                RingElement g = 1;
                g = RingMul<RingElement>(g,lambdaShares[inputs[i]][partyId],globalRingSize);

                for(int j = 0;j<inputs.size();j++)
                {
                    if(j!=i)
                    {
                        g = RingMul<RingElement>(g,blindedShares[inputs[j]],globalRingSize);
                    }
                }
                arithShare = RingAdd<RingElement>(arithShare,g,globalRingSize);
            }

            // 5. multiply constants
            for(delayedresharing::Symbol* input : multOp->inputs)
            {
                if(typeid(*input) == typeid(delayedresharing::Constant))
                {
                    delayedresharing::Constant* c = (delayedresharing::Constant*) input;
                    arithShare = RingMul<RingElement>(arithShare,(RingElement) c->value,globalRingSize);
                }
            }
            arithmeticShares[multOp] = {(partyId==0)*arithShare,(partyId==1)*arithShare};
            break;
        }

    }

    void evaluateAddArithmetic(delayedresharing::Operation* addOp,int step)
    {
        switch(step)
        {
            case 0:
            std::vector<delayedresharing::Operation*> inputs = addOp->findInputOperations();
            RingElement share = 0;
            for(delayedresharing::Operation* input : inputs)
            {
                share = RingAdd<RingElement>(share,arithmeticShares[input][partyId],globalRingSize);
            }
            if(partyId == 0)
            {
            for(int i = 0;i<addOp->inputs.size();i++)
            {
                if(typeid(*(addOp->inputs[i])) == typeid(delayedresharing::Constant))
                {
                    delayedresharing::Constant* c = (delayedresharing::Constant*) addOp->inputs[i];
                    share = RingAdd<RingElement>(share,c->value,globalRingSize);
                }
            }
            }
            arithmeticShares[addOp] = {(partyId == 0)*share,(partyId == 1)*share};
            break;
        }
    }

    void evaluateAddBlinded(delayedresharing::Operation* addOp, int step)
    {
        switch(step)
        {
            case 0:
            std::vector<delayedresharing::Operation*> inputs = addOp->findInputOperations();
            RingElement share = 0;
            for(delayedresharing::Operation* input : inputs)
            {
                share = RingAdd<RingElement>(share,blindedShares[input],globalRingSize);
            }
        
            for(int i = 0;i<addOp->inputs.size();i++)
            {
                if(typeid(*(addOp->inputs[i])) == typeid(delayedresharing::Constant))
                {
                delayedresharing::Constant* c = (delayedresharing::Constant*) addOp->inputs[i];
                share = RingAdd<RingElement>(share,c->value,globalRingSize);
                }
            }
            blindedShares[addOp] = share;
        break;
        }
    }

    void evaluateAdd(delayedresharing::Operation* addOp,int step)
    {
        switch(step)
        {
            case 0:
                if(addOp->hasSharing(delayedresharing::SharingMode::ADDITIVE))
                {
                    evaluateAddArithmetic(addOp,step);
                }
                if(addOp->hasSharing(delayedresharing::SharingMode::BLINDED))
                {
                    evaluateAddBlinded(addOp,step);
                }
            break;
        }
    }

    void evaluateOutputArithmetic(delayedresharing::Operation* opOut, int step)
    {
        delayedresharing::Operation* op = opOut->findInputOperations()[0];
        switch(step)
        {
            case 4:
                if(outputOwnerMap[opOut] == partyId)
                {
                    RingElement otherArith;
                    arithmeticShares[op][1-partyId] = 0;
                    //receive orders other arithmetic share
                    recvOrders.push_back({store: &(arithmeticShares[op].at(1-partyId))});

                }else
                {
                    // send lambda share
                    sendOrders.push_back({value: arithmeticShares[op][partyId]});
                }
                break;
            case 5:
                if(outputOwnerMap[opOut] == partyId)
                {
                RingElement output = RingAdd<RingElement>(arithmeticShares[op][partyId],arithmeticShares[op][1-partyId],globalRingSize);
                std::cout << std::to_string(output) << ", ";
                }
                break;
        }
    }

    void evaluateOutputBlinded(delayedresharing::Operation* opOut, int step)
    {
        delayedresharing::Operation* op = opOut->findInputOperations()[0];
        switch(step)
        {
            case 4:
                if(outputOwnerMap[opOut] == partyId)
                {
                    RingElement otherArith;
                    lambdaShares[op][1-partyId] = 0;
                    //receive orders other arithmetic share
                    recvOrders.push_back({store: &(lambdaShares[op].at(1-partyId))});

                }else
                {
                    // send lambda share
                    sendOrders.push_back({value: lambdaShares[op][partyId]});
                }
                break;
            case 5:
                if(outputOwnerMap[opOut] == partyId)
                {
                RingElement lambda = RingAdd<RingElement>(lambdaShares[op][partyId],lambdaShares[op][1-partyId],globalRingSize);
                RingElement output = RingAdd<RingElement>(blindedShares[op],lambda,globalRingSize);
                std::cout << std::to_string(output) << ", ";
                }
                break;
        }
    }

    void evaluateOutput(delayedresharing::Operation* op,int step)
    {
        if(op->hasSharing(delayedresharing::SharingMode::ADDITIVE))
        {
            evaluateOutputArithmetic(op,step);
        }
        else
        if(op->hasSharing(delayedresharing::SharingMode::BLINDED))
        {
            evaluateOutputBlinded(op,step);
        }
    }

    void evaluateConvArith2Blinded(delayedresharing::Operation* op,int step)
    {
        RingElement mThis = RingSub<RingElement>(arithmeticShares[op][partyId],lambdaShares[op][partyId],globalRingSize);
        switch(step)
        {
            case 2:
                // send local share of m and receive at final m location
                sendOrders.push_back({value: mThis});
                blindedShares[op] = 0;
                recvOrders.push_back({store: &blindedShares.at(op)});

            break;
            case 3:
                // add my m to the other received value
                blindedShares[op] = RingAdd<RingElement>(mThis,blindedShares[op],globalRingSize);
            break;
        }
    }

    void evaluateConvBlinded2Arith(delayedresharing::Operation* op,int step)
    {
        switch(step)
        {
            case 2:
                RingElement arith = lambdaShares[op][partyId];
                arith = RingAdd<RingElement>((partyId==0)*blindedShares[op],arith,globalRingSize);
                arithmeticShares[op] = {(partyId == 0)*arith,(partyId == 1)*arith};
            break;
        }
    }

    void evaluateConversions(delayedresharing::Operation* op,int step)
    {
        for(auto conversion : op->conversions)
        {
            if(std::get<0>(conversion) == delayedresharing::SharingMode::ADDITIVE && std::get<1>(conversion) == delayedresharing::SharingMode::BLINDED)
            {
                evaluateConvArith2Blinded(op,step);
            }else
            if(std::get<0>(conversion) == delayedresharing::SharingMode::BLINDED && std::get<1>(conversion) == delayedresharing::SharingMode::ADDITIVE)
            {
                evaluateConvBlinded2Arith(op,step);
            }
        }
    }
    

    void transmit()
    {
        int sendWidth = sendOrders.size()*globalRingWidth;
        int recvWidth = recvOrders.size()*globalRingWidth;
        
        int sSize = std::ceil((double) sendWidth / 8);
        int rSize = std::ceil((double) recvWidth / 8);

        bytesTransferredOnline += sSize;
        bytesTransferredOnline += rSize;

        if(globalRingWidth > 8)
        {
        std::vector<RingElement> toSend;
        std::vector<RingElement> received(recvOrders.size());
        for(const auto & sendOrder : sendOrders)
        {
            toSend.push_back(sendOrder.value);
        }
        if(partyId == 0)
        {
            if(sendOrders.size() > 0)
                coproto::sync_wait(partyChannel.send(toSend));
            if(recvOrders.size() > 0)
                coproto::sync_wait(partyChannel.recv(received));
        }else
        {
            if(recvOrders.size() > 0)
                coproto::sync_wait(partyChannel.recv(received));
            if(sendOrders.size() > 0)
                coproto::sync_wait(partyChannel.send(toSend));
        }
        for(int i = 0;i<recvOrders.size();i++)
        {
            *(recvOrders[i].store) = received[i];
        }
        sendOrders.clear();
        recvOrders.clear();
        }else
        {
        std::vector<osuCrypto::u8> toSend;
        std::vector<osuCrypto::u8> received(recvOrders.size());
        for(const auto & sendOrder : sendOrders)
        {
            toSend.push_back((osuCrypto::u8) sendOrder.value);
        }
        if(partyId == 0)
        {
            if(sendOrders.size() > 0)
                coproto::sync_wait(partyChannel.send(toSend));
            if(recvOrders.size() > 0)
                coproto::sync_wait(partyChannel.recv(received));
        }else
        {
            if(recvOrders.size() > 0)
                coproto::sync_wait(partyChannel.recv(received));
            if(sendOrders.size() > 0)
                coproto::sync_wait(partyChannel.send(toSend));
        }
        for(int i = 0;i<recvOrders.size();i++)
        {
            *(recvOrders[i].store) = received[i];
        }
        sendOrders.clear();
        recvOrders.clear();
        }
    }

    int ResharingDepth(delayedresharing::Operation* op,std::map<delayedresharing::Symbol*,int>* depthMap)
    {
        if(depthMap->count(op))
        {
            return (*depthMap)[op];
        }else
        {
            int ownDepth = 0;
            int childDepth = 0;
            for(const auto& childOp : op->findInputOperations())
            {

                ownDepth = std::max(ownDepth, (int) (childOp->conversions.size() > 0));
                
                childDepth = std::max(childDepth,ResharingDepth(childOp,depthMap));
            }
            int depth =  ownDepth + childDepth;
            (*depthMap)[op] = depth;
            return depth;
        }
    }

    virtual void run()
    {
        std::cout << "\n" << "==== Party: " << partyId << " starting online phase ====" << "\n";
        // default protocol is not a real thing
        
        for(const auto& operation : operations)
        {
            blindedShares[operation] = 0;
            arithmeticShares[operation] = {0,0};
        }
        int transmitRound = 0;

        std::vector<std::vector<delayedresharing::Operation*>> operationLayers;
        std::map<delayedresharing::Symbol*,int> depthMap;
        int max_layer = 0;
        std::map<int,std::vector<delayedresharing::Operation*>> layers;
        for(const auto& operation : operations)
        {
            // operation->SymbolDepth(&depthMap) also works but is less optimal for having resharings sent in batch
            int operationLayer = ResharingDepth(operation,&depthMap);
            //int operationLayer = operation->SymbolDepth(&depthMap);
            max_layer = std::max(max_layer,operationLayer);
            if(!layers.count(operationLayer))
            {
                layers[operationLayer] = {operation};
            }else
            {
                layers[operationLayer].push_back(operation);
            }

        }
        for(int i = 0;i<=max_layer;i++)
        {
            if(layers.count(i))
            {
                operationLayers.push_back(layers[i]);
            }
        }

        bool firstOutput = true;
        for(const auto& operationLayer : operationLayers)
        {
            for(int step = 0;step<6;step++)
            {
                int i = 1;
                for(const auto& operation : operationLayer)
                {
                    switch(operation->operationType)
                    {
                    case delayedresharing::OperationType::INPUT:
                    evaluateInput(operation,step);
                    break;
                    case delayedresharing::OperationType::MULT:
                    evaluateMult(operation,step);
                    break;
                    case delayedresharing::OperationType::ADD:
                    evaluateAdd(operation,step);
                    break;
                    case delayedresharing::OperationType::OUTPUT:
                    if(firstOutput)
                    {
                        std::cout << "Outputs: " << "\n";
                        firstOutput = false;
                    }
                    evaluateOutput(operation,step);
                    break;
                    }

                    evaluateConversions(operation,step);
                    i++;
                }
                // message exchange
                transmit();
            }
        }
        std::cout << "\n";
        reportCommunication("Online");
        std::cout << "\nBytes transferred offline: " << bytesTransferredOffline << "\n";
        std::cout << "\nBytes transferred online: " << bytesTransferredOnline << "\n";
        int bytesTransferredTotal = bytesTransferredOffline + bytesTransferredOnline;

        std::cout << "\nBytes transferred total: " << bytesTransferredTotal << "\n";
        coproto::sync_wait(partyChannel.close());
    }


};

}