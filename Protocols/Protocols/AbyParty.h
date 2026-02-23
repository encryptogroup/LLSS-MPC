#pragma once
#include "MaskedParty.h"
#include "libOTe/Vole/Silent/SilentVoleReceiver.h"
#include "libOTe/Vole/Silent/SilentVoleSender.h"

namespace delayedresharing {


class AbyParty : public MaskedParty {
private:


public:
    // personal prng
    osuCrypto::PRNG personalPRNG;

    std::vector<SendOrder> sendOrders;
        
    std::vector<ReceiveOrder> recvOrders;

    AbyParty(int party, std::string circuitFile,std::string otherPartyIP) : MaskedParty(party,circuitFile,otherPartyIP){

            personalPRNG = osuCrypto::PRNG(osuCrypto::toBlock(party));
    }

    //note this implementation requires a three party setup, as two party setting does not have the allPRNG
    virtual void computeFreshLambdaShare(delayedresharing::Operation* operation)
    {
        if(inputOwnerMap.count(operation))
        {
            if(inputOwnerMap[operation] == partyId)
            {
                // this party owns this input value: use party helper prng and all seed
                RingElement l_this = DrawRingElement(personalPRNG,globalRingSize);
                RingElement l_other = DrawRingElement(partyPRNG,globalRingSize);
                if((partyId == 0))
                {
                    lambdaShares[operation] = {l_this,l_other};
                }else
                {
                    lambdaShares[operation] = {l_other,l_this};
                }
            }else
            {
                // other party owns this input value: use all party seed
                RingElement l = DrawRingElement(partyPRNG,globalRingSize);
                lambdaShares[operation] = {(partyId == 0)*l,(partyId == 1)*l};
            }
        }else
        {
            // this value is part of the computation and not owned by anyone: use party helper prng
            RingElement l = DrawRingElement(personalPRNG,globalRingSize);
            lambdaShares[operation] = {(partyId == 0)*l,(partyId == 1)*l};
        }
    }

    virtual void computeGammaShares()
    {
        int numVole = 0;
        for(int i = 0;i<operations.size();i++)
        {
            if(operations[i]->operationType == delayedresharing::OperationType::MULT)
            {
                numVole += 2;
            }
        }
        if(partyId == 0)
        {
            
			// A = B + C * delta
			osuCrypto::AlignedUnVector<RingElement> B(numVole);
			RingElement delta = 0;
            // = prng.get();


			osuCrypto::SilentVoleSender<RingElement, RingElement> sender;
			sender.mLpnMultType = osuCrypto::DefaultMultType;
			sender.configure(numVole);

			// perform the OTs and write the random OTs to msgs.
			coproto::sync_wait(sender.silentSend(delta, B, personalPRNG, partyChannel));
        }else
        {
            
			osuCrypto::AlignedUnVector<RingElement> C(numVole);
			osuCrypto::AlignedUnVector<RingElement> A(numVole);

			osuCrypto::SilentVoleReceiver<RingElement, RingElement> receiver;
			receiver.mLpnMultType = osuCrypto::DefaultMultType;
			receiver.configure(numVole);
			// perform  numVole random OTs, the results will be written to msgs.
			coproto::sync_wait(receiver.silentReceive(C, A, personalPRNG, partyChannel));
        }
    }
    

    virtual void setup()
    {
        Protocol::setup();

        
        reportCommunication("Setup");
    }


};

}