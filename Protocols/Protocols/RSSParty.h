#pragma once
#include "Protocols/Protocol.h"

#include <cryptoTools/Common/Timer.h>

namespace delayedresharing {

template <typename R> class RSSParty : public Protocol {
public:
  using V = decltype(R().v);

  std::map<int, std::vector<R>> sendQueues;
  std::map<int, std::vector<R*>> recvQueues;

  // prng shared between this party and helper
  osuCrypto::PRNG myPRNG;
  osuCrypto::PRNG otherPRNG;

  // this is the channel connecting us to the helper party port for party 0 is
  int num_parties = 3;
  osuCrypto::Channel channels[3];

  std::map<delayedresharing::Symbol *, std::tuple<R, R>> replicatedShares;
  std::map<delayedresharing::Symbol *, R> additiveShares;
  std::queue<R> zeroCorrelatedShares;

  int partyId;
  int inputNumber = 0;

  std::string aIP;
  std::string bIP;

  
  std::map<delayedresharing::Operation *, std::vector<R>> outputShares;
  std::map<delayedresharing::Operation*, R> resultValue;

  RSSParty(int party, std::string circuitFile, std::string otherPartyIP,
           std::string helperPartyIP)
      : Protocol(circuitFile) {
    static_assert(std::is_base_of<DomainValue, R>::value,
                  "Error: T must be a subtype of Domain Value.");
    this->partyId = party;
    this->aIP = otherPartyIP;
    this->bIP = helperPartyIP;
    for (int i = 1; i < num_parties; i++) {
      sendQueues[(i + num_parties) % num_parties] = {};
      recvQueues[(i + num_parties) % num_parties] = {};
    }
  }

  void runInputReplicated(delayedresharing::Operation *op) {
    inputNumber++;

    // P_D
    if (inputOwnerMap[op] == partyId) {
      R v(batchSize);
      // TODO set for all
      v.v[0] = inputs[op];

      // todo: sample randomly
      R r0(batchSize);
      R r1(batchSize);

      R rd = v - (r0 + r1);

      sendQueues[(partyId + 1) % num_parties].push_back(rd);
      replicatedShares[op] = {rd, r0};
    }
    

    // P_D+1
    if  (( ( inputOwnerMap[op] + 1 ) % num_parties ) == partyId) {
      R x(batchSize);
      R a(batchSize);
      replicatedShares[op] = {x, a};
      recvQueues[inputOwnerMap[op]].push_back(
          &std::get<0>(replicatedShares[op]));
    }

    // P_D-1
    if  (( ( inputOwnerMap[op] - 1 + num_parties ) % num_parties ) == partyId) {
      R x(batchSize);
      R a(batchSize);
      replicatedShares[op] = {x, a};
    }
  }

  void runInputAdditive(delayedresharing::Operation *op) {

    if (inputOwnerMap[op] == partyId) {
      R v(batchSize);
      // TODO set for all
      v.v[0] = inputs[op];

      additiveShares[op] = v - createCorrelatedRandomness();
    } else {
      additiveShares[op] = createCorrelatedRandomness();
    }
  }

  void runAdditionAdditive(delayedresharing::Operation *op) {
    R r(batchSize);
    auto inputs = op->findInputOperations();
    for (auto input : inputs) {
      r = r + additiveShares[input];
    }

    int value = 0;
    for (auto input : op->inputs) {
      if (delayedresharing::Constant *c = dynamic_cast<delayedresharing::Constant *>(input)) {
        value += c->value;
      }
    }

    additiveShares[op] = r.addConst((partyId == 0) * value);
  }

  void runAdditionReplicated(delayedresharing::Operation *op) {
    auto inputs = op->findInputOperations();

    R x(batchSize);
    R a(batchSize);
    int value = 0;
    
    for (auto input : op->inputs) {
      if (delayedresharing::Constant *c = dynamic_cast<delayedresharing::Constant *>(input)) {
        value += c->value;
      }
    }

    for (auto input : inputs) {
      //std::cout << "Replicated Addition input: " << input->toSymbol() << "\n";
      x = x + std::get<0>(replicatedShares[input]);
      a = a + std::get<1>(replicatedShares[input]);
    }
    replicatedShares[op] = {x.addConst(value), a.addConst(value)};
  }
  
  void runMultiplicationReplicated(delayedresharing::Operation *op) {
  
      auto inputs = op->findInputOperations();
      int value = 1;
      for(auto input : op->inputs)
      {
        if(delayedresharing::Constant* c = dynamic_cast<delayedresharing::Constant*>(input))
        {
          value *= c->value;
        }
      }

      replicatedShares[op] = {std::get<0>(replicatedShares[inputs[0]]).multConst(value),
        std::get<1>(replicatedShares[inputs[0]]).multConst(value)};
  }

    void runMultiplicationAdditive(delayedresharing::Operation *op) {
  
      auto inputs = op->findInputOperations();
      int value = 1;
      for(auto input : op->inputs)
      {
        if(delayedresharing::Constant* c = dynamic_cast<delayedresharing::Constant*>(input))
        {
          value *= c->value;
        }
      }
      
      additiveShares[op] = additiveShares[inputs[0]].multConst(value);
  }

  // See araki et al. mult step 1
  void runMultiplicationReplicatedAdditive(delayedresharing::Operation *op) {
    R r(batchSize);
    auto inputs = op->findInputOperations();
    
    r = std::get<0>(replicatedShares[inputs[0]]) *
            std::get<0>(replicatedShares[inputs[1]]) +
        std::get<0>(replicatedShares[inputs[0]]) *
            std::get<1>(replicatedShares[inputs[1]]) +
        std::get<1>(replicatedShares[inputs[0]]) *
            std::get<0>(replicatedShares[inputs[1]]);

    additiveShares[op] = r;
  }

  // see araki mult step 2
  void runRerandomizeAdditive(delayedresharing::Operation *op) {
    // add 0 correlation onto all shares
    // create for every rerandomization =  # outputsharings in additive + #
    // resharings additive to replicated
    additiveShares[op] = additiveShares[op] + createCorrelatedRandomness();
  }

  void runReshareAdditiveToReplicated(delayedresharing::Operation *op) {
    // TODO check hash map if value has been rerandomized before
    runRerandomizeAdditive(op);

    // send additive shares
    R r(batchSize);

    std::tuple<R, R> share(additiveShares[op], r);
    replicatedShares[op] = share;
    switch (partyId) {
    case 0:
      recvQueues[2].push_back(&std::get<1>(replicatedShares[op]));
      sendQueues[1].push_back(additiveShares[op]);
      break;

    case 1:

      recvQueues[0].push_back(&std::get<1>(replicatedShares[op]));
      sendQueues[2].push_back(additiveShares[op]);
      break;

    case 2:
      sendQueues[0].push_back(additiveShares[op]);
      recvQueues[1].push_back(&std::get<1>(replicatedShares[op]));
      break;
    }
  }

  void runReshareReplicatedToAdditive(delayedresharing::Operation *op) {
    //std::cout << "Upgrading replicated to additive for " << op->toSymbol()
    //          << "\n";
    additiveShares[op] = std::get<0>(replicatedShares[op]);
  }


  void runOutputReplicated(delayedresharing::Operation *op) {
    // TODO: this should support the case when input is not operation but
    // constant
    auto share = replicatedShares[op->findInputOperations()[0]];

    if (outputOwnerMap[op] != partyId) {
      if(partyId == ( (outputOwnerMap[op] + 1) % num_parties) )
      {
        sendQueues[outputOwnerMap[op]].push_back(std::get<0>(share));
      }
    } else {
      outputShares[op] = {std::get<0>(share),std::get<1>(share)};
      R otherShare(batchSize);
      outputShares[op].push_back(std::move(otherShare));
      
      int from = (partyId + 1) % num_parties;
      recvQueues[from].push_back(
            &(outputShares[op][num_parties-1]));
    }
  }

  void runOutputReplicatedAfterMessage(delayedresharing::Operation *op) {
    if (outputOwnerMap[op] == partyId) {
      R result(batchSize);
      for (const auto share : outputShares[op]) {
        result = result + share;
      }
      resultValue[op] = result;
    }
  }

  void runOutputAdditive(delayedresharing::Operation *op) {
    // TODO: this should support the case when input is not operation but
    // constant
    auto share = additiveShares[op->findInputOperations()[0]];

    if (outputOwnerMap[op] != partyId) {
      sendQueues[outputOwnerMap[op]].push_back(share);
    } else {
      outputShares[op] = {share};
      for (int i = 1; i < num_parties; i++) {
        R otherShare(batchSize);
        outputShares[op].push_back(std::move(otherShare));
      }
      for (int i = 1; i < num_parties; i++) {
        int from = (partyId + i) % num_parties;
        recvQueues[from].push_back(
            &(outputShares[op][i]));
      }
    }
  }

  void runOutputAdditiveAfterMessage(delayedresharing::Operation *op) {
    if (outputOwnerMap[op] == partyId) {
      R result(batchSize);
      for (const auto share : outputShares[op]) {
        result = result + share;
      }
      resultValue[op] = result;
    }
  }

  // setup information

  int randomnessSampled = 0;
  // see Araki: create 0 correlation
  R createCorrelatedRandomness() {
    randomnessSampled++;
    return R::sample(&myPRNG,batchSize)-R::sample(&otherPRNG,batchSize);
  }

  virtual void setup() {

    // prepare keys:
    osuCrypto::block myKey = myPRNG.getSeed();

    osuCrypto::block otherKey;
    if (partyId == 2) {
      this->channels[(partyId - 1 + num_parties) % num_parties].send(myKey);
      this->channels[(partyId + 1 + num_parties) % num_parties].recv(otherKey);
    } else {
      this->channels[(partyId + 1 + num_parties) % num_parties].recv(otherKey);
      this->channels[(partyId - 1 + num_parties) % num_parties].send(myKey);
    }
    //std::cout << "received block: " << otherKey << "\n";

    otherPRNG = osuCrypto::PRNG(otherKey, sizeof(osuCrypto::block) +
                                              sizeof(osuCrypto::u64));
  }

  void workSend(int other) {

        if (sendQueues[other].size() > 0) {
          R sendData;
          for (const auto &sendValue : sendQueues[other]) {
            sendData.add(sendValue);
          }
          //std::cout << partyId << " -> " << other << ": " << sendData.v.size() << "\n";
          this->channels[other].send(sendData.v);
        }
  }

  void workRecv(int other) {
        if (recvQueues[other].size() > 0) {
          int size = recvQueues[other].size();
          R receiveData(size * batchSize);
          //std::cout << partyId << " <- " << other << ": " << size << "\n";
          this->channels[other].recv(receiveData.v);
          std::vector<R> receivedValues =
              receiveData.get(recvQueues[other].size());
          for (int i = 0; i < recvQueues[other].size(); i++) {
            R* location = recvQueues[other][i];
            location->v = receivedValues[i].v;
          }
        }
  }

  void transmit() {
    for (int i = 1; i < num_parties; i++) {
      
      int direction = 1;
      if (partyId == 0)
      {
        direction = -1;
      }
      int other = (partyId + direction*i + num_parties) % num_parties;

      bool whoSendsFirst = partyId < other;
      if (whoSendsFirst) {
        workSend(other);
        workRecv(other);
      } else {
        workRecv(other);
        workSend(other);
      }

      sendQueues[other] = {};
      recvQueues[other] = {};
    }
  }

  int InteractionDepth(delayedresharing::Operation *op,
                       std::map<delayedresharing::Symbol *, int> *depthMap) {
    if (depthMap->count(op)) {
      return (*depthMap)[op];
    } else {
      int maxDepth = 0;
      for (const auto &childOp : op->findInputOperations()) {
        int depthOffset = 0;
        if (
          childOp->operationType == delayedresharing::OperationType::INPUT &&
           childOp->operationSharings.contains({delayedresharing::SharingMode::PLAIN,delayedresharing::SharingMode::BLINDED})
          ) {
          depthOffset = 1;
        }
        
        if (childOp->conversions.contains({delayedresharing::SharingMode::ADDITIVE,
                                           delayedresharing::SharingMode::BLINDED})) {
          depthOffset += 1;
        }
        maxDepth = std::max(maxDepth,
                            InteractionDepth(childOp, depthMap) + depthOffset);
      }
      (*depthMap)[op] = maxDepth;
      return maxDepth;
    }
  }

  std::vector<std::vector<delayedresharing::Operation *>> GetLayers() {
    std::vector<std::vector<delayedresharing::Operation *>> operationLayers;
    std::map<delayedresharing::Symbol *, int> depthMap;
    int max_layer = 0;
    std::map<int, std::vector<delayedresharing::Operation *>> layers;
    for (const auto &operation : operations) {
      if(operation->operationType == delayedresharing::OperationType::OUTPUT)
      {
        continue;
      }
      int operationLayer = InteractionDepth(operation, &depthMap);
      //std::cout << "Operation: " << operation->toSymbol() << " Layer: " << operationLayer << "\n";
      max_layer = std::max(max_layer, operationLayer);
      if (!layers.count(operationLayer)) {
        layers[operationLayer] = {operation};
      } else {
        layers[operationLayer].push_back(operation);
      }
    }
    for (int i = 0; i <= max_layer; i++) {
      if (layers.count(i)) {
        operationLayers.push_back(layers[i]);
      }
    }

      std::vector<delayedresharing::Operation *> outputLayer;
    for (const auto &operation : operations) {
      if(operation->operationType == delayedresharing::OperationType::OUTPUT)
      {
        outputLayer.push_back(operation);
      }
    }
    operationLayers.push_back(outputLayer);

    return operationLayers;
  }

  // NOTE:  we currently reuse the blinded sharing on our circuit description to
  // mean replicated sharing

  virtual void run() {
    std::vector<std::vector<delayedresharing::Operation *>> layers = GetLayers();
    //std::cout << "Layers: " << layers.size() << "\n";
    for (auto layer : layers) {
      for (delayedresharing::Operation *op : layer) {
        switch (op->operationType) {
        case delayedresharing::OperationType::INPUT:
        
          if (op->operationSharings.contains(
                  {delayedresharing::SharingMode::PLAIN,
                   delayedresharing::SharingMode::ADDITIVE})) {
          runInputAdditive(op);
          }else
          if (op->operationSharings.contains(
                  {delayedresharing::SharingMode::PLAIN,
                   delayedresharing::SharingMode::BLINDED})) {
          runInputReplicated(op);
          }
          break;
        case delayedresharing::OperationType::ADD:
          if (op->operationSharings.contains(
                  {delayedresharing::SharingMode::ADDITIVE,
                   delayedresharing::SharingMode::ADDITIVE})) {
            runAdditionAdditive(op);
          }
          if (op->operationSharings.contains({delayedresharing::SharingMode::BLINDED,
                                              delayedresharing::SharingMode::BLINDED})) {
            runAdditionReplicated(op);
          }
          break;
        case delayedresharing::OperationType::MULT:
          if (op->operationSharings.contains(
                  {delayedresharing::SharingMode::BLINDED,
                   delayedresharing::SharingMode::ADDITIVE})) {
            runMultiplicationReplicatedAdditive(op);
          }
          
          if (op->operationSharings.contains(
                  {delayedresharing::SharingMode::BLINDED,
                   delayedresharing::SharingMode::BLINDED})) {
            runMultiplicationReplicated(op);
          }

          if (op->operationSharings.contains(
                  {delayedresharing::SharingMode::ADDITIVE,
                   delayedresharing::SharingMode::ADDITIVE})) {
            runMultiplicationAdditive(op);
          }
          break;
        case delayedresharing::OperationType::OUTPUT:

          if (op->operationSharings.contains({delayedresharing::SharingMode::ADDITIVE,
                                              delayedresharing::SharingMode::PLAIN})) {
            // todo: this does not work if we have a constant
            auto inputOps = op->findInputOperations();
            if (inputs.size() > 0) {
              runRerandomizeAdditive(inputOps[0]);
            }
            runOutputAdditive(op);
          } else if (op->operationSharings.contains(
                         {delayedresharing::SharingMode::BLINDED,
                          delayedresharing::SharingMode::PLAIN})) {
            runOutputReplicated(op);
          }
          break;
        }
        for (auto conversion : op->conversions) {
          if (std::get<0>(conversion) == delayedresharing::SharingMode::BLINDED &&
                     std::get<1>(conversion) ==
                         delayedresharing::SharingMode::ADDITIVE && replicatedShares.count(op)) {
            runReshareReplicatedToAdditive(op);
          }
        }
      }

      transmit();

      for (auto op : layer) {
                  
        assert( (op->conversions.size() == 0) || op->operationType != delayedresharing::OperationType::OUTPUT);
        for (auto conversion : op->conversions) {
          //std::cout << SharingModeName(std::get<0>(conversion)) << "->"
          //          << SharingModeName(std::get<1>(conversion)) << "\n";
          if (std::get<0>(conversion) == delayedresharing::SharingMode::ADDITIVE &&
              std::get<1>(conversion) == delayedresharing::SharingMode::BLINDED) {
            runReshareAdditiveToReplicated(op);
          }
        }
      }

      transmit();

      for (delayedresharing::Operation *op : layer) {
        switch (op->operationType) {
        case delayedresharing::OperationType::OUTPUT:

          if (op->operationSharings.contains({delayedresharing::SharingMode::ADDITIVE,
                                              delayedresharing::SharingMode::PLAIN})) {
            runOutputAdditiveAfterMessage(op);
          } else if (op->operationSharings.contains(
                         {delayedresharing::SharingMode::BLINDED,
                          delayedresharing::SharingMode::PLAIN})) {
            runOutputReplicatedAfterMessage(op);
          }
          break;
        }
        for (auto conversion : op->conversions) {
          if (std::get<0>(conversion) == delayedresharing::SharingMode::BLINDED &&
                     std::get<1>(conversion) ==
                         delayedresharing::SharingMode::ADDITIVE && replicatedShares.count(op)) {
            runReshareReplicatedToAdditive(op);
          }
        }

      }
    }
  }

  virtual void launch() {
    osuCrypto::IOService ios(2);
    ios.showErrorMessages(true);

    osuCrypto::Timer timer;
    int me = partyId % num_parties;

    myPRNG = osuCrypto::PRNG(ios.getRandom(),
                             sizeof(osuCrypto::block) + sizeof(osuCrypto::u64));

    int i = 1;
    int direction = (me == 2) ? 1 : -1;

    int other = (partyId + direction * i + num_parties) % num_parties;
    int ancil = 0;
    
    // true = i lead
    bool who_leads = (me + 1) % num_parties == other;
    
    std::string ip;
    if (who_leads) {
      
      ip = aIP;
      ancil = 10 * me + other;
    } else {
      ip = bIP;
      ancil = 10 * other + me;
    }
    std::string connection = ip + ":" + std::to_string(13500 + ancil);
    auto mode = osuCrypto::SessionMode::Server;
    if (!who_leads) {
      mode = osuCrypto::SessionMode::Client;
    }
    osuCrypto::Session sessionA = osuCrypto::Session(ios, connection, mode);

    osuCrypto::Channel channelA = sessionA.addChannel("main");
    channelA.waitForConnection();

    this->channels[(other) % 3] = channelA;


    i = 2;
    direction = (me == 2) ? 1 : -1;
    ip = (i > 1) ? aIP : bIP;

    other = (partyId + direction * i + num_parties) % num_parties;
    ancil = 0;
    who_leads = (me + 1) % num_parties == other;
    if (who_leads) {
      ip = aIP;
      ancil = 10 * me + other;
    } else {
      ip = bIP;
      ancil = 10 * other + me;
    }
    connection = ip + ":" + std::to_string(13500 + ancil);
    mode = osuCrypto::SessionMode::Server;
    if (!who_leads) {
      mode = osuCrypto::SessionMode::Client;
    }
    osuCrypto::Session sessionB = osuCrypto::Session(ios, connection, mode);

    osuCrypto::Channel channelB = sessionB.addChannel("main");
    channelB.waitForConnection();

    this->channels[(other) % 3] = channelB;




  timer.reset();
  timer.setTimePoint("before protocol");

    int send = channelA.getTotalDataSent() +
  channelB.getTotalDataSent() ;
    int recv = channelA.getTotalDataRecv() +
   channelB.getTotalDataRecv();
    // start setupPhase
    setup();


    send = channelA.getTotalDataSent() +
  channelB.getTotalDataSent()  - send;
    recv = channelA.getTotalDataRecv() +
  channelB.getTotalDataRecv()  - recv;

    run();

    
    send = channelA.getTotalDataSent() +
  channelB.getTotalDataSent()  - send;
    recv = channelA.getTotalDataRecv() +
  channelB.getTotalDataRecv()  - recv;


    std::cout << "     total: " << (send+recv) << std::endl;

    
    timer.setTimePoint("after protocol");
    auto it = timer.mTimes.end();
    it--;
    it--;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(timer.mTimes.back().first - it->first);
    double duration_ms = duration.count() / 1000.0;
    runtime = duration_ms;
    
    for(const auto operation : operations)
    {
      if(operation->operationType == delayedresharing::OperationType::OUTPUT)
      {
        if (outputOwnerMap[operation] == partyId) {
          //resultValue[operation].print();
        }
      }
    }
    channelA.close();
    channelB.close();

    sessionA.stop();
    sessionB.stop();

    ios.stop();

   
  }
};

} // namespace delayedresharing
