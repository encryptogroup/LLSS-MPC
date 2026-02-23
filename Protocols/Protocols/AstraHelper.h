#pragma once

#include "Protocols/MaskedProtocol.h"

namespace delayedresharing {

class AstraHelper : public MaskedProtocol {

public:
  // channel connecting to online parties: channel i goes to party i
  coproto::Socket partyChannel;

  // prng shared by all parties
  osuCrypto::PRNG allPRNG;

  // prng to parties
  std::vector<osuCrypto::PRNG> partyPRNGs;

  std::vector<SendOrder> sendOrders;

  std::vector<ReceiveOrder> recvOrders;

  AstraHelper(std::string circuitFile, std::string partyAIP)
      : MaskedProtocol(circuitFile) {
    std::cout << "Helper is Host with IPs: " << partyAIP << "\n";
    partyChannel = coproto::asioConnect(partyAIP + ":13556", true);
    sockets.push_back(partyChannel);
    std::cout << "Connected to online parties\n";
    allPRNG = osuCrypto::PRNG(osuCrypto::toBlock(7));
    partyPRNGs.push_back(osuCrypto::PRNG(osuCrypto::toBlock(6)));
    partyPRNGs.push_back(osuCrypto::PRNG(osuCrypto::toBlock(5)));
  }

  virtual void computeFreshLambdaShare(delayedresharing::Operation *operation) {
    if (inputOwnerMap.count(operation)) {
      // operation is input operation owned by one of the parties
      RingElement l_0;
      RingElement l_1;
      if (inputOwnerMap[operation] == 0) {
        l_0 = DrawRingElement(partyPRNGs[0], globalRingSize);
        l_1 = DrawRingElement(allPRNG, globalRingSize);
      } else {
        l_0 = DrawRingElement(allPRNG, globalRingSize);
        l_1 = DrawRingElement(partyPRNGs[1], globalRingSize);
      }

      lambdaShares[operation] = {l_0, l_1};
    } else {
      // this value is part of the computation and not owned by anyone: use
      // party helper prng
      lambdaShares[operation] = {
          DrawRingElement(partyPRNGs[0], globalRingSize),
          DrawRingElement(partyPRNGs[1], globalRingSize)};
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
        // compute lambda product, subtract according to randomness
        RingElement gamma = 1;
        for (auto input : inputSet) {
          // recombine lambda
          RingElement lambda = RingAdd<RingElement>(lambdaShares[input][0], lambdaShares[input][1], globalRingSize);

          gamma = RingMul<RingElement>(gamma, lambda, globalRingSize);
        }

        //  3. if send to party 0 the share, select randomness with party 1
        RingElement gamma1 = DrawRingElement(partyPRNGs[1], globalRingSize);
        RingElement gamma0 =
            RingSub<RingElement>(gamma, gamma1, globalRingSize);
        sendOrders.push_back({value : gamma0});
        gammaShares[index] = {gamma0, gamma1};
      }
    }
  }

  void transmitSetup() {
    if (sendOrders.size() > 0) {
      std::vector<RingElement> toSend;
      for (const auto &sendOrder : sendOrders) {
        toSend.push_back(sendOrder.value);
      }
      std::cout << "sending: " << toSend.size() << " elements"
                << "\n";

      coproto::sync_wait(partyChannel.send(toSend));
      sendOrders.clear();
    }
  }

  virtual void setup() {
    Protocol::setup();
    transmitSetup();

    reportCommunication("Setup");
    // self destroy
    coproto::sync_wait(partyChannel.close());
  }

  // helper literally should not do anything in online phase
  virtual void run() { return; }
};

} // namespace delayedresharing