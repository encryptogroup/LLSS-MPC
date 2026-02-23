#pragma once
#include "Optimizations/OptimizationPass.h"
#include <queue>
#include <unordered_set>

namespace delayedresharing {

    /* A ShareAssignment method must assign vor every node
       1. at least one sharings,
       2. for every Operation at least one operationSharings
       3. needed conversions


       Currently Share Assignments only operate on whole GTNs, this could be modified however
     */
    class ShareAssignment : public OptimizationPass {
        public:

void removeElementFromQueue(std::queue<delayedresharing::Operation *> &q,
                            delayedresharing::Operation *target) {
  std::queue<delayedresharing::Operation *> temp;

  while (!q.empty()) {
    if (q.front() != target) {
      temp.push(q.front());
    }
    q.pop();
  }

  while (!temp.empty()) {
    q.push(temp.front());
    temp.pop();
  }
}

bool augmentFlowPath(
    delayedresharing::FlowNode *s, delayedresharing::FlowNode *t,
    std::map<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *,
             delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *>
        *oppositeEdge) {
  std::vector<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *>
      pathEdges;
  std::vector<delayedresharing::FlowNode *> pathNodes;
  std::queue<std::vector<delayedresharing::FlowNode *>> queue;
  std::unordered_set<delayedresharing::FlowNode *> visited;

  queue.push({s});

  while (!queue.empty()) {
    std::vector<delayedresharing::FlowNode *> currentPath = queue.front();
    queue.pop();

    delayedresharing::FlowNode *currentNode = currentPath.back();

    if (currentNode == t) {
      pathNodes = currentPath;
      break;
    }

    if (!visited.count(currentNode)) {
      visited.insert(currentNode);
      for (const auto &n : currentNode->next) {
        if (n->capacity - n->flow > 0) {
          std::vector<delayedresharing::FlowNode *> modifiedPath;
          modifiedPath = currentPath;
          modifiedPath.push_back(n->b);
          queue.push(modifiedPath);
        }
      }
    }
  }
  // it is ok to translate from list of nodes to list of edges like this,
  // because when there are two edges from node a to b they allways have infty
  // capacity so can be added
  if (pathNodes.size() > 0) {
    for (int i = 0; i < (pathNodes.size() - 1); i++) {
      for (const auto &n : pathNodes[i]->next) {
        if (n->b == pathNodes[i + 1]) {
          pathEdges.push_back(n);
          break;
        }
      }
    }
  }
  if (pathEdges.size() > 0) {
    int minOpenCapacity = std::numeric_limits<int>::max();
    for (const auto &pathEdge : pathEdges) {
      minOpenCapacity =
          std::min((pathEdge->capacity - pathEdge->flow), minOpenCapacity);
    }

    for (const auto &pathEdge : pathEdges) {
      pathEdge->flow += minOpenCapacity;
      ((*oppositeEdge)[pathEdge])->flow -= minOpenCapacity;
    }
    return true;
  }
  return false;
}

delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *connect(
    delayedresharing::FlowNode *a, delayedresharing::FlowNode *b, int capacity,
    std::vector<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *>
        *edges,
    std::map<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *,
             delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *>
        *backflow) {
  delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *connection =
      new delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>();

  connection->a = a;
  connection->b = b;
  a->next.push_back(connection);
  connection->capacity = capacity;

  edges->push_back(connection);

  delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>
      *backconnection =
          new delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>();

  backconnection->a = b;
  backconnection->b = a;
  b->next.push_back(backconnection);
  backconnection->capacity = 0;
  (*backflow)[connection] = backconnection;
  (*backflow)[backconnection] = connection;

  return connection;
}

            void clearSharing( delayedresharing::GeneralizedTermNetwork* termNetwork) {
                for(auto genTerm : termNetwork->generalizedTerms)
                {
                    clearSharing(genTerm);
                }
            }
            
            void clearSharing( delayedresharing::GeneralizedTerm* genTerm) {
                    for(auto node : genTerm->nodes)
                    {
                        node->sharings.clear();
                        node->conversions.clear();
                        if(delayedresharing::Operation* op = dynamic_cast<delayedresharing::Operation*>(node))
                        {
                            op->operationSharings.clear();
                        }
                    }
            }

            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork)
            {
                return;
            }

            // Make sure that sharings are consistent across the network
            virtual bool consistencyCheck( delayedresharing::GeneralizedTermNetwork* termNetwork)
            {
                for(auto const& term : termNetwork->generalizedTerms)
                {
                    for(auto const& node : term->nodes)
                    {
                        if(typeid(*node) == typeid(delayedresharing::Operation))
                        {
                            delayedresharing::Operation* op = (delayedresharing::Operation*) node;
                            for(auto const& opSharing : op->operationSharings)
                            {
                                int i_count = 0;
                                for(auto inputOp : op->findInputOperations())
                                {
                                    if(!inputOp->sharings.count(std::get<0>(opSharing)))
                                    {
                                        std::cout << "Inconsistency found!" << "\n";
                                        std::cout << "Operation: " << op->toSymbol() << " Input# "<< i_count <<": " << inputOp->toSymbol() <<  " has missing sharing: " << SharingModeName(std::get<0>(opSharing)) << "\n";
                                        std::cout << "That input has " << inputOp->reportShareAssignment() << "\n";
                                        std::cout << "Term: " << op->render() << "\n";
                                        std::cout << "Conflicting InputTerm: " << inputOp->render() << "\n";
                                        termNetwork->toDot("dotFiles/faulty_sharing.dot");
                                        return false;
                                    }
                                    i_count++;
                                }
                            }
                        }
                    }
                }
                return true;
            }
    };

    class AstraShareAssignment : public ShareAssignment {
        public:
            AstraShareAssignment()
            {
                name = "AstraShareAssignment";
            }
            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork);
    };
    class GreedyShareAssignment : public ShareAssignment {
        public:
            GreedyShareAssignment()
            {
                name = "GreedyShareAssignment";
            }
            virtual void apply( delayedresharing::GeneralizedTerm* genTerm);
            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork);
    };
    class BoundaryShareAssignment : public ShareAssignment {
        public:
            bool assignInputOutput = true;
            enum Model {
                MASKED,
                REPLICATED,
                SHAMIR
            };
            Model costModel = Model::MASKED;
            
            int numParties = 3;
            int threshold = 2;
            BoundaryShareAssignment()
            {
                name = "BoundaryShareAssignment";
            }
            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork);
    };
    class OfflineBoundaryShareAssignment : public ShareAssignment {
        public:
            
            OfflineBoundaryShareAssignment()
            {
                name = "OfflineBoundaryShareAssignment";
            }
            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork);
    };
    
    // this sets every share to plain shares with a single ring width everywhere
    class PlainShareAssignment : public ShareAssignment {
        public:
            PlainShareAssignment()
            {
                name = "PlainShareAssignment";
            }
            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork);
    };

}