
#include <GeneralizedTerms/GeneralizedTermNetwork.h>

std::vector<delayedresharing::Operation*> delayedresharing::GeneralizedTermNetwork::OperationsInTopologicalOrder()
{
  std::vector<delayedresharing::Operation*> operations;
  std::map<delayedresharing::Symbol*,int> depthMap;
  for(auto generalizedTerm : generalizedTerms)
  {
    for(auto node : generalizedTerm->nodes)
    {
      if(typeid(*node) == typeid(delayedresharing::Operation))
      {
        delayedresharing::Operation* o = (delayedresharing::Operation*) node;
        operations.push_back(o);
      }
    }
  }
  std::sort(operations.begin(),operations.end(),[&depthMap,this](Operation* a, Operation* b) {
    if(a->operationType == delayedresharing::OperationType::INPUT)
    {
      if(b->operationType == a->operationType)
      {
        return std::find(this->generalizedTerms[0]->nodes.begin(),this->generalizedTerms[0]->nodes.end(),a) < std::find(this->generalizedTerms[0]->nodes.begin(),this->generalizedTerms[0]->nodes.end(),b);
      }else
      return true;
    }
    if( b->operationType == delayedresharing::OperationType::OUTPUT)
    {
      if(a->operationType == b->operationType)
      {
        return std::find(this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.begin(),this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.end(),a) < std::find(this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.begin(),this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.end(),b);
      }else
      return true;
    }

    if(a->operationType == delayedresharing::OperationType::OUTPUT)
    {
      if(b->operationType == a->operationType)
      {
        return std::find(this->generalizedTerms[0]->nodes.begin(),this->generalizedTerms[0]->nodes.end(),a) < std::find(this->generalizedTerms[0]->nodes.begin(),this->generalizedTerms[0]->nodes.end(),b);
      }else
      return false;
    }
    if( b->operationType == delayedresharing::OperationType::INPUT)
    {
      if(a->operationType == b->operationType)
      {
        return std::find(this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.begin(),this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.end(),a) < std::find(this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.begin(),this->generalizedTerms[this->generalizedTerms.size()-1]->nodes.end(),b);
      }else
      return false;
    }

    return a->SymbolDepth(&depthMap) < b->SymbolDepth(&depthMap);
  });
  return operations;
}

delayedresharing::ComputationHyperGraph* delayedresharing::GeneralizedTermNetwork::extractComputationGraph()
{

  auto computationGraph = new delayedresharing::ComputationHyperGraph();
  int node_counter = 0;
  std::map< delayedresharing::Operation *, int> lookup;

  // We do not wish to partition input and output genTerm with input or output operations
  for (int i = 1;i<generalizedTerms.size()-1;i++) {
    delayedresharing::GeneralizedTerm* genTerm = generalizedTerms[i];
    for (auto node : genTerm->nodes) {
      if (typeid(*node) == typeid( delayedresharing::Operation)) {
          delayedresharing::Operation* operation = (( delayedresharing::Operation *)node);
          computationGraph->num_nodes++;
          lookup[operation] = node_counter;
          computationGraph->index[node_counter] = operation;
          node_counter++;
      }
    }
  }

  // We do not wish to partition input and output genTerm
  for (int i = 1;i<(generalizedTerms.size()-1);i++) {
    delayedresharing::GeneralizedTerm* genTerm = generalizedTerms[i];
    for (auto node : genTerm->nodes) {

      if (typeid(*node) == typeid( delayedresharing::Operation)) {

        delayedresharing::Operation* operation = (delayedresharing::Operation*) node;

          // for every use: collect recusively until you hit an operation again,

          auto nextOperations = operation->findNextOperations();
          // construct edges from this list and current operation
          delayedresharing::HyperEdge<int, int>* newEdge = new delayedresharing::HyperEdge<int, int>();
          newEdge->members.push_back(lookup[operation]);
          int numberAdded = 0;
          for (delayedresharing::Operation* nextOp : nextOperations) {
            if (nextOp->operationType != delayedresharing::OperationType::OUTPUT) {
              newEdge->members.push_back(lookup[(delayedresharing::Operation*) nextOp]);
              numberAdded++;
            }
          }
          // TODO, this might need to be a different assignment, based upon
          // operation, should be based on costmodel
          newEdge->annotation =
              1 + (operation->operationType == delayedresharing::OperationType::MULT);
          if(numberAdded>0)
          {
            computationGraph->edges.push_back(newEdge);
          }else
          {
            delete newEdge;
          }
      }
    }
  }

  return computationGraph;
}


void delayedresharing::GeneralizedTermNetwork::assignDefUses() {
  for ( delayedresharing::GeneralizedTerm* genTerm : generalizedTerms) {
    for(delayedresharing::Symbol* node : genTerm->nodes)
      node->defUses.clear();
  }
  for ( delayedresharing::GeneralizedTerm* genTerm : generalizedTerms) {
    genTerm->assignDefUses();
  }
}




void delayedresharing::validateNodeSet(std::vector<delayedresharing::Symbol*> nodes)
{
  std::cout << "running validation" << "\n";
  int i = 0;
    for(auto node : nodes)
    {
        if(typeid(*node) == typeid(delayedresharing::Operation))
        {
            delayedresharing::Operation* o = (delayedresharing::Operation*) node;
            for(auto input : o->inputs)
            {
              if(input == nullptr || (input == 0))
              {
                std::cout << "Operation: " << i << " has null pointer input\n";
                assert(false);
              }
            }
        }
      i++;
    }
}


void delayedresharing::GeneralizedTermNetwork::assignSharings()
{
  for(auto term : generalizedTerms)
  {
    for(auto node : term->nodes)
    {
      node->assignSharings();
    }
  }
}


void delayedresharing::GeneralizedTermNetwork::reportShareAssignment()
{
    for(auto genTerm : generalizedTerms)
    {
        std::cout << "==GenTerm==" << "\n";
        for(auto node : genTerm->nodes)
        {
          std::cout   << node->render() << " Shares: " << node->reportShareAssignment() << "\n";
        }
    }
}


// Fully clone this Generalized Term Network with fresh symbols
            delayedresharing::GeneralizedTermNetwork* delayedresharing::GeneralizedTermNetwork::clone()
            {
                delayedresharing::GeneralizedTermNetwork* clonedNetwork = new delayedresharing::GeneralizedTermNetwork();

                // creating the lookup here forces all variables to be freshly cloned

                std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>* freshSymbolLookup = new std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>();

                delayedresharing::GeneralizedTerm* newInputs = generalizedTerms[0]->clone(freshSymbolLookup);


                clonedNetwork->generalizedTerms.push_back(newInputs);
                clonedNetwork->inputs = newInputs;

              
                // note every fresh output variable gets its input assigned as an input variable in some later generalized term
                for(int i = 1;i<(generalizedTerms.size()-1);i++)
                {

                    delayedresharing::GeneralizedTerm* clonedGeneralizedTerm = generalizedTerms[i]->clone(freshSymbolLookup);

                    clonedNetwork->generalizedTerms.push_back(clonedGeneralizedTerm);
                }


                
                delayedresharing::GeneralizedTerm* newOutputs = generalizedTerms[generalizedTerms.size()-1]->clone(freshSymbolLookup);

                clonedNetwork->generalizedTerms.push_back(newOutputs);
                clonedNetwork->outputs = newOutputs;


                return clonedNetwork;
            }