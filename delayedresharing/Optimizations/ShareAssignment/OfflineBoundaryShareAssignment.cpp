#include "GeneralizedTerms/HyperGraph.h"
#include "Optimizations/ShareAssignment/ShareAssignment.h"
#include <limits.h>
#include <queue>
#include <unordered_set>



void delayedresharing::OfflineBoundaryShareAssignment::apply(
    delayedresharing::GeneralizedTermNetwork *network) {

  network->assignDefUses();


  int costResharing;
  int costLowInput;
  int costHighInput;
  int costLowOutput;
  int costHighOutput;
  int costRerand;

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
      
      int baseline = 0;
    // baseline fixes -> 1 per mult
    for (const auto& op : opsInTopo) 
    {
          if (op->operationType == delayedresharing::OperationType::MULT && op->findInputOperations().size() > 1) {
            baseline++;
          }
    }
    std::cout << "Baseline: " << baseline << "\n";

    // create flow problem differently: find the bottleneck between multiplications and upgrades as present in the input share assignment.

    std::queue<delayedresharing::Operation *> relevant_operations;
    std::map<delayedresharing::Operation *, bool> represented;
    std::map<delayedresharing::Operation *, bool> relevant;
    std::map<delayedresharing::Operation *, delayedresharing::FlowNode*> representative;
    std::map<delayedresharing::Operation*, bool> has_cutable_edge;

    // only create flow nodes for operations between interactive multiplications and lower domain outputs/upgrades
    for (const auto& op : opsInTopo) {
          bool op_is_relevant = false;
          bool op_is_represented = false;
          
          bool left = false;
          bool right = false;
          bool create_upgrade_dummy = false;
          
          if (op->operationType == delayedresharing::OperationType::MULT && op->findInputOperations().size() > 1) {
              op_is_represented = true;
              op_is_relevant = true;
              left = true;
          }

          for(const auto& inputOp : op->findInputOperations())
          {
              if(relevant[inputOp])
              {
                op_is_represented = true;
                op_is_relevant = true;
                break;
              }
          }

          if(op->hasConversionToMode(delayedresharing::SharingMode::BLINDED))
          {
            op_is_represented = true;
            op_is_relevant = false;
            create_upgrade_dummy = true;
          }

          if(op->operationType == delayedresharing::OperationType::OUTPUT)
          {

            if(op->hasOperationSharing({delayedresharing::SharingMode::ADDITIVE,delayedresharing::PLAIN}))
            {
              op_is_represented = true;
              op_is_relevant = false;
              right = true;
            }
          }


          if(op_is_represented)
          {
            // create representative in flow problem
            
            delayedresharing::FlowNode *flownode = new delayedresharing::FlowNode();
            representative[op] = flownode;
            nodes.push_back(flownode);
            has_cutable_edge[op] = false;

            if(!right)
            {
              // lawler transformation
              delayedresharing::FlowNode *dummynode =
              new delayedresharing::FlowNode();
              nodes.push_back(dummynode);
              representative[op] = dummynode;
              connect(flownode, dummynode, 1, &edges, &backflow);
              has_cutable_edge[op] = true;
              dummies[op].push_back(flownode);
              dummies[op].push_back(dummynode);
            }
            for(const auto& inputOp : op->findInputOperations())
            {
              if(represented[inputOp])
              {
                connect(representative[inputOp],flownode,infty,&edges, &backflow);
              }
            }
            if(left)
            {
              connect(mainSource,flownode,infty,&edges,&backflow);
            }
            if(right)
            {
              connect(flownode,mainSink,infty,&edges,&backflow);
            }
            if(create_upgrade_dummy)
            {
              delayedresharing::FlowNode *upgradedummynode =
              new delayedresharing::FlowNode();
              nodes.push_back(upgradedummynode);
              connect(representative[op],upgradedummynode,infty,&edges,&backflow);
              connect(upgradedummynode,mainSink,infty,&edges,&backflow);
            }
          }


          represented[op] = op_is_represented;
          relevant[op] = op_is_relevant;
    }





  clearSharing(network);

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

 
  int fixes = 0;
  // TODO: ist das unique

    for (delayedresharing::Operation *op : opsInTopo) {


        // if edge from dummyl to dummyout was cut: put fix gate // count
        if(has_cutable_edge[op])
        {
        if (reachable[dummies[op][0]] && !reachable[dummies[op][1]]) {
          fixes++;
        }
        }
    }


    std::cout << "Fixes: " << fixes << std::endl;
  

  network->assignSharings();

  for (const auto &node : nodes) {
    delete node;
  }
  for (const auto &edge : edges) {
    delete edge;
  }
  // if (!consistencyCheck(network)) {

  //   assert(false);
  //   std::cout << "There was a consistency error!\n";
  //   exit(1);
  // }
}
