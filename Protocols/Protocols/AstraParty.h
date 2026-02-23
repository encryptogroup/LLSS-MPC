#pragma once
#include "MaskedParty.h"

namespace delayedresharing {

class AstraParty : public MaskedParty {
private:
public:
  // prng shared by all parties and helper
  osuCrypto::PRNG allPRNG;

  // prng shared between this party and helper
  osuCrypto::PRNG helperPRNG;

  // this is the channel connecting us to the helper party port for party 0 is
  // 13556 for party 1 13557
  coproto::Socket helperChannel;

  std::vector<SendOrder> sendOrders;

  std::vector<ReceiveOrder> recvOrders;

  AstraParty(int party, std::string circuitFile, std::string otherPartyIP, std::string helperPartyIP) : MaskedParty(party, circuitFile, otherPartyIP) {
    if (party == 0) {
      this->helperChannel = coproto::asioConnect(helperPartyIP + ":" + std::to_string(13556 + party), false);
      std::cout << "Connected to helper "<< helperPartyIP << "\n";
      sockets.push_back(this->helperChannel);
    }
    // key currently = binary of which parties included: 111 = all parties 110 =
    // helper and party 0 101 = helper and party 1
    allPRNG = osuCrypto::PRNG(osuCrypto::toBlock(7));
    int helperSeed = 0;
    if (partyId == 0) {
      helperSeed = 6;
    } else {
      helperSeed = 5;
    }
    helperPRNG = osuCrypto::PRNG(osuCrypto::toBlock(helperSeed));
  }

  // note this implementation requires a three party setup, as two party setting
  // does not have the allPRNG
  virtual void computeFreshLambdaShare(delayedresharing::Operation *operation) {
    if (inputOwnerMap.count(operation)) {
      if (inputOwnerMap[operation] == partyId) {
        // this party owns this input value: use party helper prng and all seed
        RingElement l_this = DrawRingElement(helperPRNG, globalRingSize);
        RingElement l_other = DrawRingElement(allPRNG, globalRingSize);
        if ((partyId == 0)) {
          lambdaShares[operation] = {l_this, l_other};
        } else {
          lambdaShares[operation] = {l_other, l_this};
        }
      } else {
        // other party owns this input value: use all party seed
        RingElement l = DrawRingElement(allPRNG, globalRingSize);
        lambdaShares[operation] = {(partyId == 0) * l, (partyId == 1) * l};
      }
    } else {
      // this value is part of the computation and not owned by anyone: use
      // party helper prng
      RingElement l = DrawRingElement(helperPRNG, globalRingSize);
      lambdaShares[operation] = {(partyId == 0) * l, (partyId == 1) * l};
    }
  }

  virtual void computeGammaShare(delayedresharing::Operation *operation) {
    // 1. get every subset of inputs with at least 2 elements
    std::vector<std::vector<delayedresharing::Operation *>> inputSets =
        InputSets(operation);
    // for each subset:
    for (auto inputSet : inputSets) {
      //  2. check if gammaShares dont allready have input vectors
      size_t index = hash_array(inputSet);
      if (!gammaShares.count(index)) {
        RingElement gamma = 0;

        if (partyId == 1) {
          // draw fresh
          gamma = DrawRingElement(helperPRNG, globalRingSize);
          gammaShares[index] = {0, gamma};
        } else {
          // receive from helper channel
          gammaShares[index] = {0, 0};
          recvOrders.push_back({store : &gammaShares[index].at(0)});
        }
      }
    }
  }

  void transmitSetup() {
    if (recvOrders.size() > 0) {
      std::vector<RingElement> received(recvOrders.size());

      std::cout << "receiving: " << received.size() << " elements"
                << "\n";
      coproto::sync_wait(helperChannel.recv(received));

      
      bytesTransferredOffline += received.size()*globalRingWidth;
      for (int i = 0; i < recvOrders.size(); i++) {
        *(recvOrders[i].store) = received[i];
      }
      recvOrders.clear();
    }
  }

  virtual void setup() {
    Protocol::setup();

    if (partyId == 0) {
      transmitSetup();
      reportCommunication("Setup");


      // kill channel to helper
      coproto::sync_wait(helperChannel.close());
    }
  }
};

} // namespace delayedresharing