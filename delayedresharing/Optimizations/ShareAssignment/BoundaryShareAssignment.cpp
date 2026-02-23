#include "GeneralizedTerms/HyperGraph.h"
#include "Optimizations/ShareAssignment/ShareAssignment.h"
#include <limits.h>
#include <queue>
#include <unordered_set>



void delayedresharing::BoundaryShareAssignment::apply(
    delayedresharing::GeneralizedTermNetwork *network) {

  network->assignDefUses();
  clearSharing(network);
  network->toDot("dotFiles/in.dot");


  int costResharing;
  int costLowInput;
  int costHighInput;
  int costLowOutput;
  int costHighOutput;
  int costRerand;
  if(costModel == Model::SHAMIR)
  {
    costResharing = 2 * numParties - threshold - 1;
    costLowInput = 0;
    costHighInput = numParties - threshold;
    costLowOutput = numParties - 1;
    costHighOutput = threshold - 1;
    costRerand = 0;
  }
  if (costModel == Model::REPLICATED){
    costResharing = 3;
    costLowInput = 0;
    costHighInput = 1;
    costLowOutput = 2;
    costHighOutput = 1;
    costRerand = 0;

  }

  // ABY2.0
  
  if (costModel == Model::MASKED){
    costResharing = 2;
    costLowInput = 0;
    costHighInput = 1;
    costLowOutput = 1;
    costHighOutput = 1;
    costRerand = 0;
  }

  // create transformed graph
  std::vector<delayedresharing::FlowNode *> nodes;

  // connection lookup 0 = left node 1 = right node
  std::map<delayedresharing::Operation *,
           std::vector<delayedresharing::FlowNode *>>
      nodeConnectionLookup;
  std::map<delayedresharing::Operation *,
           std::vector<delayedresharing::FlowNode *>>
      dummies;
  std::map<delayedresharing::FlowNode *, delayedresharing::Operation *>
      reverseNodeLookup;
  std::vector<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *>
      edges;

  std::map<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *,
           delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *>
      backflow;

  delayedresharing::FlowNode *mainSource = new delayedresharing::FlowNode();
  delayedresharing::FlowNode *mainSink = new delayedresharing::FlowNode();
  nodes.push_back(mainSource);
  nodes.push_back(mainSink);

  int infty = std::numeric_limits<int>::max();

  
  std::vector<delayedresharing::Operation *> opsInTopo =
      network->OperationsInTopologicalOrder();

  if (assignInputOutput) {

    for (delayedresharing::GeneralizedTerm *genTerm :
         network->generalizedTerms) {
      for (auto node : genTerm->nodes) {
        if (delayedresharing::Operation *op =
                dynamic_cast<delayedresharing::Operation *>(node)) {
          delayedresharing::FlowNode *dummy1;
          delayedresharing::FlowNode *dummy2;
          delayedresharing::FlowNode *dummyout;
          switch (op->operationType) {
          case delayedresharing::OperationType::INPUT: {
            // construct:
            //   S -- infty --> dummy1 -- (cost of high sharing - cost of low
            //   sharing) --> dummy2 -- ( cost of conversion) --> dummyout -- >
            //   rest of circuit
            dummy1 = new delayedresharing::FlowNode();
            dummy2 = new delayedresharing::FlowNode();
            dummyout = new delayedresharing::FlowNode();

            nodes.push_back(dummy1);
            nodes.push_back(dummy2);
            nodes.push_back(dummyout);

            nodeConnectionLookup[op].push_back(dummyout);
            reverseNodeLookup[dummy1] = op;
            reverseNodeLookup[dummy2] = op;
            reverseNodeLookup[dummyout] = op;

            dummies[op].push_back(dummy1);
            dummies[op].push_back(dummy2);
            dummies[op].push_back(dummyout);

            connect(mainSource, dummy1, infty, &edges, &backflow);

            connect(dummy1, dummy2, costHighInput - costLowInput, &edges,
                    &backflow);

            connect(dummy2, dummyout, costResharing, &edges, &backflow);
            break;
          }
          case delayedresharing::OperationType::OUTPUT: {
            // construct:
            //   rest of circuit -- infty --> dummy1 -- (cost of low reconstruct
            //   - cost of high reconstruct) --> dummy2 -- infty -- > T
            delayedresharing::FlowNode *dummy1 =
                new delayedresharing::FlowNode();
            delayedresharing::FlowNode *dummy2 =
                new delayedresharing::FlowNode();
            nodes.push_back(dummy1);
            nodes.push_back(dummy2);

            // at position 1 needed
            nodeConnectionLookup[op].push_back(dummy1);
            nodeConnectionLookup[op].push_back(dummy1);
            reverseNodeLookup[dummy1] = op;
            reverseNodeLookup[dummy2] = op;

            dummies[op].push_back(dummy1);
            dummies[op].push_back(dummy2);

            connect(dummy1, dummy2, costLowOutput - costHighOutput, &edges,
                    &backflow);
            connect(dummy2, mainSink, infty, &edges, &backflow);
            break;
          }
          default: {
            // insert regular
            delayedresharing::FlowNode *dummyl;
            delayedresharing::FlowNode *dummyr;
            bool interactive =
                (op->operationType == delayedresharing::OperationType::MULT) &&
                (op->findInputOperations().size() > 1);
            if (interactive) {
              dummyl = new delayedresharing::FlowNode();
              dummyr = new delayedresharing::FlowNode();
              nodes.push_back(dummyl);
              nodes.push_back(dummyr);
            } else {
              dummyl = new delayedresharing::FlowNode();
              dummyr = dummyl;
              nodes.push_back(dummyl);
            }

            delayedresharing::FlowNode *dummyout =
                new delayedresharing::FlowNode();
            nodes.push_back(dummyout);
            connect(dummyl, dummyout, costResharing, &edges, &backflow);
            nodeConnectionLookup[op].push_back(dummyout);
            nodeConnectionLookup[op].push_back(dummyr);
            reverseNodeLookup[dummyl] = op;
            reverseNodeLookup[dummyr] = op;
            reverseNodeLookup[dummyout] = op;

            dummies[op].push_back(dummyl);
            dummies[op].push_back(dummyr);
            dummies[op].push_back(dummyout);

            if (interactive) {
              connect(mainSource, dummyl, infty, &edges, &backflow);
              connect(dummyr, mainSink, infty, &edges, &backflow);
            }

            break;
          }
          }

          for (const auto &input : op->findInputOperations()) {
            auto a = nodeConnectionLookup[input][0];
            auto b = nodeConnectionLookup[op][1];
            connect(a, b, infty, &edges, &backflow);
          }
        }
      }
    }

  } else {
    // create flow problem differently: no dummies for input output choice,
    // place upgrades everywhere between first mults and outputs

    std::queue<delayedresharing::Operation *> relevant_operations;
    std::map<delayedresharing::Operation *, bool> visited;

    for (delayedresharing::GeneralizedTerm *genTerm :
         network->generalizedTerms) {
      for (auto node : genTerm->nodes) {
        if (delayedresharing::Operation *op =
                dynamic_cast<delayedresharing::Operation *>(node)) {
          visited[op] = false;
          switch (op->operationType) {
          case delayedresharing::OperationType::INPUT: {
            break;
          }
          case delayedresharing::OperationType::OUTPUT: {
            relevant_operations.push(op);
            break;
          }
          case delayedresharing::OperationType::MULT: {
            if (op->findInputOperations().size() > 1) {
              relevant_operations.push(op);
            }
            break;
          }
          }
        }
      }
    }

    std::set<delayedresharing::Operation *> flowGraphOperations;
    while (relevant_operations.size() > 0) {
      delayedresharing::Operation *op = relevant_operations.front();
      relevant_operations.pop();
      flowGraphOperations.insert(op);
      visited[op] = true;
      auto next_operations = op->findNextOperations();
      for (const auto &next_op : next_operations) {
        if (!visited[next_op]) {
          {
            visited[op] = true;
            relevant_operations.push(next_op);
          }
        }
      }
    }

    for (const auto &op : opsInTopo) {
      if(!flowGraphOperations.count(op))
      {
        continue;
      }
      assert(op->operationType != delayedresharing::OperationType::INPUT);

      if(op->operationType == delayedresharing::OperationType::OUTPUT)
      {
        // ONLY RIGHT SIDE
        delayedresharing::FlowNode *outnode = new delayedresharing::FlowNode();
        reverseNodeLookup[outnode] = op;
        nodeConnectionLookup[op].push_back(outnode);
        nodeConnectionLookup[op].push_back(outnode);
        nodes.push_back(outnode);
        dummies[op].push_back(outnode);
        connect(outnode, mainSink, infty, &edges, &backflow);
      }
      else if(op->operationType == delayedresharing::OperationType::MULT && (op->findInputOperations().size() > 1)) {
        // LEFT AND RIGHT SIDE
        delayedresharing::FlowNode *leftnode = new delayedresharing::FlowNode();
        delayedresharing::FlowNode *rightnode =
            new delayedresharing::FlowNode();
        delayedresharing::FlowNode *dummynode =
            new delayedresharing::FlowNode();
        reverseNodeLookup[leftnode] = op;
        reverseNodeLookup[rightnode] = op;
        reverseNodeLookup[dummynode] = op;
        nodeConnectionLookup[op].push_back(dummynode);
        nodeConnectionLookup[op].push_back(rightnode);
        nodes.push_back(leftnode);
        nodes.push_back(rightnode);
        nodes.push_back(dummynode);

        dummies[op].push_back(leftnode);
        dummies[op].push_back(dummynode);
        dummies[op].push_back(rightnode);
        connect(rightnode, mainSink, infty, &edges, &backflow);
        connect(mainSource, leftnode, infty, &edges, &backflow);
        connect(leftnode, dummynode, costResharing, &edges, &backflow);

      }
      else {
        // ONLY LEFT SIDE
        delayedresharing::FlowNode *leftnode = new delayedresharing::FlowNode();
        delayedresharing::FlowNode *dummynode =
            new delayedresharing::FlowNode();
        reverseNodeLookup[leftnode] = op;
        nodeConnectionLookup[op].push_back(dummynode);
        nodeConnectionLookup[op].push_back(leftnode);
        nodes.push_back(leftnode);
        nodes.push_back(dummynode);

        dummies[op].push_back(leftnode);
        dummies[op].push_back(dummynode);
        connect(leftnode, dummynode, costResharing, &edges, &backflow);
      }
      // wire up inputs
      for (const auto &input : op->findInputOperations()) {
        if (nodeConnectionLookup.count(input)) {
          auto a = nodeConnectionLookup[input][0];
          auto b = nodeConnectionLookup[op][1];
          connect(a, b, infty, &edges, &backflow);
        }
      }
    }
  }

  // compute edge connectivity on graph with ford fulkerson

  for (const auto &e : edges) {
    e->flow = 0;
  }

  bool couldAugment = false;
  do {
    couldAugment = augmentFlowPath(mainSource, mainSink, &backflow);
  } while (couldAugment);

  // BFS on C_f

  std::queue<delayedresharing::FlowNode *> reachableCandidates;
  reachableCandidates.push(mainSource);
  std::map<delayedresharing::FlowNode *, bool> reachable;

  for (const auto &node : nodes) {
    reachable[node] = false;
  }

  while (reachableCandidates.size() > 0) {
    delayedresharing::FlowNode *reachedNode = reachableCandidates.front();
    reachableCandidates.pop();
    if (reachable[reachedNode]) {
      continue;
    }
    reachable[reachedNode] = true;
    for (const auto &nextEdge : reachedNode->next) {
      if ( //(nextEdge->capacity > 0) &&
          (nextEdge->capacity - nextEdge->flow) > 0) {
        reachableCandidates.push(nextEdge->b);
      }
    }
  }


  if (assignInputOutput) {

    // insert upgrades
    for (delayedresharing::Operation *op : opsInTopo) {

      switch (op->operationType) {

      case delayedresharing::OperationType::OUTPUT: {
        break;
      }

      case delayedresharing::OperationType::INPUT: {
        if (reachable[dummies[op][1]] && !reachable[dummies[op][2]]) {
          op->addConversion({delayedresharing::SharingMode::ADDITIVE,
                             delayedresharing::SharingMode::BLINDED});
        }
        break;
      }

      default: {
        // if edge from dummyl to dummyout was cut: put reshare

        if (reachable[dummies[op][0]] && !reachable[dummies[op][2]]) {
          op->addConversion({delayedresharing::SharingMode::ADDITIVE,
                             delayedresharing::SharingMode::BLINDED});
        }
        break;
      }
      }
    }

    for (delayedresharing::Operation *op : opsInTopo) {

      switch (op->operationType) {

      case delayedresharing::OperationType::INPUT: {
        // if edge from dummy 1 to dummy 2 was cut: put blinded input sharing
        // otherwise additive

        if (reachable[dummies[op][0]] && !reachable[dummies[op][1]]) {
          op->addOperationSharing({delayedresharing::SharingMode::PLAIN,
                                   delayedresharing::SharingMode::BLINDED});
        } else {
          op->addOperationSharing({delayedresharing::SharingMode::PLAIN,
                                   delayedresharing::SharingMode::ADDITIVE});
        }
        break;
      }

      case delayedresharing::OperationType::OUTPUT: {
        // if edge from dummy 1 to dummy 2 was cut: put additive output sharing
        // otherwise blinded

        if (reachable[dummies[op][0]] && !reachable[dummies[op][1]]) {
          op->addOperationSharing({delayedresharing::SharingMode::ADDITIVE,
                                   delayedresharing::SharingMode::PLAIN});
        } else {
          op->addOperationSharing({delayedresharing::SharingMode::BLINDED,
                                   delayedresharing::SharingMode::PLAIN});
        }
        break;
      }

      default: {

        if (op->operationType == delayedresharing::OperationType::MULT &&
            op->findInputOperations().size() > 1) {
          op->addOperationSharing({delayedresharing::SharingMode::BLINDED,
                                   delayedresharing::SharingMode::ADDITIVE});
        } else {

          bool allInputsBlinded = true;
          for (const auto &pOp : op->findInputOperations()) {
            if (!(pOp->sharings.count(
                    delayedresharing::SharingMode::BLINDED))) {
              allInputsBlinded = false;
              break;
            }
          }
          if (allInputsBlinded) {
            if (op->operationType != delayedresharing::OperationType::INPUT)
              op->addOperationSharing({delayedresharing::SharingMode::BLINDED,
                                       delayedresharing::SharingMode::BLINDED});
          } else {
            if (op->operationType != delayedresharing::OperationType::INPUT)
              op->addOperationSharing(
                  {delayedresharing::SharingMode::ADDITIVE,
                   delayedresharing::SharingMode::ADDITIVE});

            for (const auto &inputOp : op->findInputOperations()) {
              if (!(inputOp->sharings.count(
                      delayedresharing::SharingMode::ADDITIVE))) {
                inputOp->addConversion(
                    {delayedresharing::SharingMode::BLINDED,
                     delayedresharing::SharingMode::ADDITIVE});
              }
            }
          }
        }

        break;
      }
      }
    }

  } else {
    // set all inputs and outputs to high sharing, add upgrades according to
    // flow

    // insert upgrades
    for (delayedresharing::Operation *op : opsInTopo) {

      switch (op->operationType) {

      default: {
        // if edge from dummyl to dummyout was cut: put reshare
        if(dummies.count(op) > 0)
        {
        if (reachable[dummies[op][0]] && !reachable[dummies[op][1]]) {
          op->addConversion({delayedresharing::SharingMode::ADDITIVE,
                             delayedresharing::SharingMode::BLINDED});
        }
        }
        break;
      }
      }
    }

    // for nodes that do not have dummy: set to high sharing

    for (delayedresharing::Operation *op : opsInTopo) {

      switch (op->operationType) {
      case delayedresharing::OperationType::INPUT: {
        op->addOperationSharing({delayedresharing::SharingMode::PLAIN,
                                 delayedresharing::SharingMode::BLINDED});
        break;
      }
      case delayedresharing::OperationType::OUTPUT: {
        op->addOperationSharing({delayedresharing::SharingMode::BLINDED,
                                 delayedresharing::SharingMode::PLAIN});
        break;
      }
      default: {

        if (dummies.count(op) == 0) {
          op->addOperationSharing({delayedresharing::SharingMode::BLINDED,
                                   delayedresharing::SharingMode::BLINDED});

        } else {
          if (op->operationType == delayedresharing::OperationType::MULT &&
              op->findInputOperations().size() > 1) {
            op->addOperationSharing({delayedresharing::SharingMode::BLINDED,
                                     delayedresharing::SharingMode::ADDITIVE});
          } else {

            bool allInputsBlinded = true;
            for (const auto &pOp : op->findInputOperations()) {
              if (!(pOp->sharings.count(
                      delayedresharing::SharingMode::BLINDED))) {
                allInputsBlinded = false;
                break;
              }
            }
            if (allInputsBlinded) {
              if (op->operationType != delayedresharing::OperationType::INPUT)
                op->addOperationSharing(
                    {delayedresharing::SharingMode::BLINDED,
                     delayedresharing::SharingMode::BLINDED});
            } else {
              if (op->operationType != delayedresharing::OperationType::INPUT)
                op->addOperationSharing(
                    {delayedresharing::SharingMode::ADDITIVE,
                     delayedresharing::SharingMode::ADDITIVE});

              for (const auto &inputOp : op->findInputOperations()) {
                if (!(inputOp->sharings.count(
                        delayedresharing::SharingMode::ADDITIVE))) {
                  inputOp->addConversion(
                      {delayedresharing::SharingMode::BLINDED,
                       delayedresharing::SharingMode::ADDITIVE});
                }
              }
            }
          }
        }
        break;
      }
      }
    }
  }

  network->assignSharings();

  for (const auto &node : nodes) {
    delete node;
  }
  for (const auto &edge : edges) {
    delete edge;
  }
  if (!consistencyCheck(network)) {

    assert(false);
    std::cout << "There was a consistency error!\n";
    exit(1);
  }
}
