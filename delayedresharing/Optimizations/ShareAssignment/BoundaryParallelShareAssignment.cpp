#include "GeneralizedTerms/HyperGraph.h"
#include "Optimizations/ShareAssignment/ShareAssignment.h"
#include <limits.h>
#include <queue>
#include <thread>
#include <unordered_set>
/* Idea:
- Compute all connected components of local operations of GTN
- filter for those, which eventually feed into some operation that needs a
resharing i.e. a non local multiplication
- for each of them compute the min flow boundary on unit weights, place
resharings from arithmetic to blinded on the boundary
*/

bool HasCrossTerms(delayedresharing::Operation *o) {
  bool opHasCrossTerms = false;

  int variable_inputs = 0;
  variable_inputs = o->findInputOperations().size();
  switch (o->operationType) {
  case delayedresharing::OperationType::MULT:
    if (variable_inputs > 1)
      opHasCrossTerms = true;
    break;
  case delayedresharing::OperationType::DOTPRODUCT:
    if (variable_inputs > 1)
      opHasCrossTerms = true;
    break;
  }
  return opHasCrossTerms;
}

bool CrossTermInFollowing(delayedresharing::Operation *o,
                          std::map<delayedresharing::Operation *, bool> *hasCrossTerms) {
  for (auto nOp : o->findNextOperations()) {
    if (((*hasCrossTerms)[nOp]) || CrossTermInFollowing(nOp, hasCrossTerms)) {
      return true;
    }
  }
  return false;
}
bool CrossTermInPrior(delayedresharing::Operation *o,
                      std::map<delayedresharing::Operation *, bool> *hasCrossTerms) {
  if ((*hasCrossTerms)[o]) {
    return true;
  }

  for (auto pOp : o->findInputOperations()) {
    if (((*hasCrossTerms)[pOp]) || CrossTermInPrior(pOp, hasCrossTerms)) {
      return true;
    }
  }
  return false;
}

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

bool augmentFlowPath(delayedresharing::FlowNode *s, delayedresharing::FlowNode *t) {
  std::vector<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *> pathEdges;
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
      for (const auto &n : currentNode->next) {
        if ((n->capacity - n->flow) > 0) {
          std::vector<delayedresharing::FlowNode *> modifiedPath;
          modifiedPath = currentPath;
          modifiedPath.push_back(n->b);
          queue.push(modifiedPath);
        }
      }
      visited.insert(currentNode);
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
    }
    return true;
  }
  return false;
}

void SearchComponent(delayedresharing::FlowNode *n,
                     std::vector<delayedresharing::FlowNode *> *cComponent) {

  std::map<delayedresharing::FlowNode *, bool> hadComponent;

  std::queue<delayedresharing::FlowNode *> nodesToCheck;
  std::unordered_set<delayedresharing::FlowNode *> visited;
  nodesToCheck.push(n);

  while (nodesToCheck.size() > 0) {

    delayedresharing::FlowNode *current = nodesToCheck.front();
    nodesToCheck.pop();
    visited.insert(current);

    // if any child allready has a component => disolve it into our nComponent
    // and delete it
    for (const auto &next : current->next) {
      if (next->b->component != nullptr) {
        auto componentOfNext = next->b->component;

        if (componentOfNext != cComponent) {
          cComponent->insert(cComponent->end(), componentOfNext->begin(),
                             componentOfNext->end());

          // for all nodes in foreign component assign this component as the
          // correct one;
          for (const auto &componentNode : *(componentOfNext)) {
            componentNode->component = cComponent;
          }
          delete componentOfNext;
          next->b->component = cComponent;
        }
      } else {

        cComponent->push_back(next->b);
        // recurse over those neighbors which did not yet have a component
        if (!visited.count(next->b)) {
          nodesToCheck.push(next->b);
        }

        next->b->component = cComponent;
      }
    }

    hadComponent.clear();
  }
}

void delayedresharing::BoundaryShareAssignment::apply(
    delayedresharing::GeneralizedTermNetwork *network) {

  network->assignDefUses();
  clearSharing(network);

  std::vector<std::set<delayedresharing::Operation *>> localOperationGroups;

  std::map<delayedresharing::Operation *, bool> hasCrossTerms;
  std::map<delayedresharing::Operation *, bool> betweenCrossTerms;

  // cache properties of all operations

  for (auto genTerm : network->generalizedTerms) {
    for (auto node : genTerm->nodes) {
      if (typeid(*node) == typeid(delayedresharing::Operation)) {
        delayedresharing::Operation *o = (delayedresharing::Operation *)node;
        hasCrossTerms[o] = HasCrossTerms(o);
      }
    }
  }

  for (auto genTerm : network->generalizedTerms) {
    for (auto node : genTerm->nodes) {
      if (typeid(*node) == typeid(delayedresharing::Operation)) {
        delayedresharing::Operation *o = (delayedresharing::Operation *)node;
        betweenCrossTerms[o] =
            hasCrossTerms[o] || (CrossTermInPrior(o, &hasCrossTerms) &&
                                 CrossTermInFollowing(o, &hasCrossTerms));
      }
    }
  }

  std::set<delayedresharing::Operation *> localOperationGroup;
  for (const auto &genTerm : network->generalizedTerms) {
    for (const auto &node : genTerm->nodes) {
      if (typeid(*node) == typeid(delayedresharing::Operation)) {
        delayedresharing::Operation *o = (delayedresharing::Operation *)node;
        if (betweenCrossTerms[o]) {
          localOperationGroup.insert(o);
        }
      }
    }
  }

  // determine where resharings needed for each local Operation group

  // modifiedLookup[op] = {from,to};
  std::map<delayedresharing::Operation *, std::vector<delayedresharing::FlowNode *>>
      modifiedLookup;
  std::map<delayedresharing::FlowNode *, delayedresharing::Operation *> reverseLookup;

  // create transformed graph
  std::vector<delayedresharing::FlowNode *> nodes;

  std::vector<delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *> edges;

  std::set<delayedresharing::FlowNode *> starts;

  std::set<delayedresharing::FlowNode *> ends;

  std::map<delayedresharing::FlowNode *, bool> reachable;

  for (delayedresharing::Operation *op : localOperationGroup) {

    if (HasCrossTerms(op)) {
      // crossterms have seperate inbound and outbound nodes
      delayedresharing::FlowNode *a;
      delayedresharing::FlowNode *b;

      bool a_connected = false;
      // one of following operations is in region
      for (auto const &nOp : op->findNextOperations()) {
        if (localOperationGroup.count(nOp) > 0) {
          a_connected = true;
          break;
        }
      }

      bool b_connected = false;
      // one of previous operations is in region
      for (auto const &pOp : op->findInputOperations()) {
        if (localOperationGroup.count(pOp) > 0) {
          b_connected = true;
          break;
        }
      }

      if (!(a_connected || b_connected)) {
        // this crossterm node goes directly to an output, not needed for
        // problem
        continue;
      }
      if (a_connected) {
        a = new delayedresharing::FlowNode();
        nodes.push_back(a);
        reverseLookup[a] = op;
        starts.insert(a);
        reachable[a] = false;
      }
      if (b_connected) {
        b = new delayedresharing::FlowNode();
        nodes.push_back(b);
        reverseLookup[b] = op;
        ends.insert(b);
        reachable[b] = false;
      }
      modifiedLookup[op] = {a, b};
    } else {
      delayedresharing::FlowNode *a = new delayedresharing::FlowNode();
      nodes.push_back(a);
      reverseLookup[a] = op;
      reachable[a] = false;
      modifiedLookup[op] = {a, a};
    }
  }

  for (delayedresharing::Operation *op : localOperationGroup) {
    const auto &nOps = op->findNextOperations();
    bool newedge = false;
    for (delayedresharing::Operation *nOp : nOps) {
      if (localOperationGroup.count(nOp)) {
        newedge = true;
      }
    }
    if (!newedge) {
      continue;
    }
    delayedresharing::FlowNode *e_out = new delayedresharing::FlowNode();
    nodes.push_back(e_out);

    reachable[e_out] = false;

    delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *edge_connection =
        new delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>();
    edge_connection->a = modifiedLookup[op][0];
    edge_connection->b = e_out;
    // edge with 1
    edge_connection->capacity = 1;
    edges.push_back(edge_connection);
    modifiedLookup[op][0]->next.push_back(edge_connection);

    for (delayedresharing::Operation *nOp : nOps) {
      if (localOperationGroup.count(nOp)) {
        delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *edge_out =
            new delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>();
        edge_out->a = e_out;
        edge_out->b = modifiedLookup[nOp][1];
        e_out->next.push_back(edge_out);
        // edge with 1
        edge_out->capacity = std::numeric_limits<int>::max();
        edges.push_back(edge_out);
      }
    }
  }

  // seperate into components

  for (const auto &node : nodes) {
    if (node->component == nullptr) {
      // this node does not yet have a component, assign a singleton component
      // and enhance it
      std::vector<delayedresharing::FlowNode *> *component =
          new std::vector<delayedresharing::FlowNode *>();
      component->push_back(node);
      node->component = component;
    }

    SearchComponent(node, node->component);
  }

  std::unordered_set<std::vector<delayedresharing::FlowNode *> *> finalComponents;

  // collect components
  for (const auto &node : nodes) {
    finalComponents.insert(node->component);
  }

#ifdef DEBUG
  std::cout << "The Boundary Share Assignment Problem has "
            << finalComponents.size() << " components to solve\n";
#endif

  std::map<std::vector<delayedresharing::FlowNode *> *, delayedresharing::FlowNode *> sources;
  std::map<std::vector<delayedresharing::FlowNode *> *, delayedresharing::FlowNode *> sinks;
  for (const auto &componentRef : finalComponents) {
    auto component = *componentRef;
    // add source and sink node

    delayedresharing::FlowNode *mainSource = new delayedresharing::FlowNode();
    delayedresharing::FlowNode *mainSink = new delayedresharing::FlowNode();
    sources[componentRef] = mainSource;
    sinks[componentRef] = mainSink;
    for (const auto &node : component) {
      if (starts.count(node)) {
        delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *sourceEdge =
            new delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>();
        sourceEdge->a = mainSource;
        sourceEdge->b = node;
        mainSource->next.push_back(sourceEdge);
        // edge with 1
        sourceEdge->capacity = std::numeric_limits<int>::max();
        edges.push_back(sourceEdge);
      }
      if (ends.count(node)) {
        delayedresharing::FlowEdge<delayedresharing::FlowNode *, int> *sinkEdge =
            new delayedresharing::FlowEdge<delayedresharing::FlowNode *, int>();
        sinkEdge->a = node;
        sinkEdge->b = mainSink;
        node->next.push_back(sinkEdge);
        // edge with 1
        sinkEdge->capacity = std::numeric_limits<int>::max();
        edges.push_back(sinkEdge);
      }
    }
  }

  std::vector<std::thread> threads;
  for (auto &componentRef : finalComponents) {

    delayedresharing::FlowNode *mainSource = sources[componentRef];
    delayedresharing::FlowNode *mainSink = sinks[componentRef];

    threads.emplace_back([componentRef, edges, mainSource, mainSink,
                                      &reverseLookup, &reachable,
                                      &modifiedLookup, &localOperationGroup]() {
      auto component = *componentRef;

      // compute edge connectivity on graph with ford fulkerson

      for (auto &e : edges) {
        e->flow = 0;
      }

      bool couldAugment = false;
      do {
        couldAugment = augmentFlowPath(mainSource, mainSink);
      } while (couldAugment);

      // BFS on C_f

      std::queue<delayedresharing::FlowNode *> reachableCandidates;
      reachableCandidates.push(mainSource);

      while (reachableCandidates.size() > 0) {
        delayedresharing::FlowNode *reachedNode = reachableCandidates.front();
        reachableCandidates.pop();
        if (reverseLookup.count(reachedNode) > 0) {
          reachable[reachedNode] = true;
        }
        for (auto &nextEdge : reachedNode->next) {
          if ((nextEdge->capacity - nextEdge->flow) > 0) {
            reachableCandidates.push(nextEdge->b);
          }
        }
      }

      std::map<delayedresharing::Operation *, bool> reachableOp;
      // add conversions
      for (auto &[flownode, is_reachable] : reachable) {
        if (reverseLookup.count(flownode)) {
          delayedresharing::Operation *op = reverseLookup[flownode];
          reachableOp[op] = is_reachable;
          if (is_reachable) {
            // this is an operation that was reachable
            for (auto &nop : op->findNextOperations()) {
              // if one of the to nodes of its results was not reachable, i.e.
              // is in group and needs resharing
              if (localOperationGroup.count(nop)) {
                if (!reachable[modifiedLookup[nop][1]]) {
                  // reshare
                  op->conversions.insert({delayedresharing::SharingMode::ADDITIVE, delayedresharing::SharingMode::BLINDED});
                  break;
                }
              }
            }
          }
        }
      }

      // add operation sharings for local operations
      for (const auto &node : component) {

        if (reverseLookup.count(node)) {
          delayedresharing::Operation *o = reverseLookup[node];
          if (!HasCrossTerms(o)) {
            if (reachableOp[o]) {
              o->operationSharings.insert(
                  {delayedresharing::SharingMode::ADDITIVE, delayedresharing::SharingMode::ADDITIVE});
            } else {
              // blinded and arithmetic (this uses extra memory but should be
              // negligible)
              // TODO maybe test if arithmetic one can be removed here
              o->operationSharings.insert({delayedresharing::SharingMode::BLINDED, delayedresharing::SharingMode::BLINDED});
              o->operationSharings.insert(
                  {delayedresharing::SharingMode::ADDITIVE, delayedresharing::SharingMode::ADDITIVE});
            }
          } else {
            // assign operationSharings for multiplications and dot products
            // with crossterms: blinded to arithmetic
            o->operationSharings.insert({delayedresharing::SharingMode::BLINDED, delayedresharing::SharingMode::ADDITIVE});
          }
        }
      }
    }

    );
  }

  for (auto &t : threads) {
    t.join();
  }

  for (const auto &node : nodes) {
    delete node;
  }
  for (const auto &edge : edges) {
    delete edge;
  }

  // input operations allways blinded
  for (auto node : network->inputs->nodes) {
    if (typeid(*node) == typeid(delayedresharing::Operation)) {
      ((delayedresharing::Operation *)node)
          ->operationSharings.insert({delayedresharing::SharingMode::BLINDED, delayedresharing::SharingMode::BLINDED});
    }
  }
  int max_depth = 0;
  std::map<delayedresharing::Symbol *, int> depthMap;
  // output operations for now allways arithmetic if no output sharing given yet
  for (auto node : network->outputs->nodes) {
    if (typeid(*node) == typeid(delayedresharing::Operation)) {
      delayedresharing::Operation *op = ((delayedresharing::Operation *)node);
      if (op->operationSharings.size() == 0) {
        op->operationSharings.insert({delayedresharing::SharingMode::ADDITIVE, delayedresharing::SharingMode::ADDITIVE});
      }
      max_depth = std::max(max_depth, op->SymbolDepth(&depthMap));
    }
  }
  // todo: this propagation should have actual parameter
  for (int d = 0; d < max_depth; d++) {
    // add sharings to every local operation with paths to inputs
    for (const auto &term : network->generalizedTerms) {
      for (const auto &node : term->nodes) {
        if (typeid(*node) == typeid(delayedresharing::Operation)) {
          delayedresharing::Operation *op = (delayedresharing::Operation *)node;
          if (!(localOperationGroup.count(op))) {
            bool allInputsBlinded = true;
            for (const auto &pOp : op->findInputOperations()) {
              if (!(pOp->sharings.count(delayedresharing::SharingMode::BLINDED))) {
                allInputsBlinded = false;
                break;
              }
            }
            if(allInputsBlinded)
            {
                if(op->operationType != delayedresharing::OperationType::INPUT)
                    op->operationSharings.insert({delayedresharing::SharingMode::BLINDED,delayedresharing::SharingMode::BLINDED});
            }else
            {
                if(op->operationType != delayedresharing::OperationType::INPUT)
                    op->operationSharings.insert({delayedresharing::SharingMode::ADDITIVE,delayedresharing::SharingMode::ADDITIVE});
            }
          }
        }
      }
    }
    network->assignSharings();

    // for every operation that has only arithmetic sharing and and input is
    // blinded, add conversion
    for (const auto &term : network->generalizedTerms) {
      for (const auto &node : term->nodes) {
        if (typeid(*node) == typeid(delayedresharing::Operation)) {
          delayedresharing::Operation *op = (delayedresharing::Operation *)node;
          for (const auto &pOp : op->findInputOperations()) {

            if (!pOp->sharings.count(delayedresharing::SharingMode::ADDITIVE)) {
              pOp->conversions.insert({delayedresharing::SharingMode::BLINDED, delayedresharing::SharingMode::ADDITIVE});
            }
          }
        }
      }
    }
    network->assignSharings();
  }

  // cleanup: if an output operation has two different sharings just use one of
  // them

  for (auto node : network->outputs->nodes) {
    if (delayedresharing::Operation *outputOp =
            dynamic_cast<delayedresharing::Operation *>(node)) {

      if (outputOp->operationSharings.size() > 1) {
        outputOp->operationSharings.erase(outputOp->operationSharings.begin());
      }
    }
  }

  for (const auto &createdComponent : finalComponents) {
    delete createdComponent;
  }
  assert(consistencyCheck(network));
}
