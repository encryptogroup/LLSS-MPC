#include <GeneralizedTerms/GeneralizedTermNetwork.h>


#include <sstream>
#include <iomanip>




void delayedresharing::GeneralizedTerm::toDot(std::string path) {

  // Create and open a text file
  std::ofstream dotFile(path);

  // Write to the file
  dotFile << "digraph G {" << "\n";

  int nodeCounter = 0;
  std::map< delayedresharing::Symbol*,int> nodeNumber;

    for(auto node : this->nodes)
    {
        if(!nodeNumber.count(node))
        {
            nodeNumber[node] = nodeCounter;
            dotFile << "        n"<<nodeCounter << delayedresharing::NodeText(node) <<"\n";
            nodeCounter++;
        }
    }


  // render all edges
    for(auto node : this->nodes)
    {
      if(typeid(*node) == typeid( delayedresharing::Operation))
      {
        delayedresharing::Operation* o = ( delayedresharing::Operation*) node;
        int i = 1;
        for(auto in : o->inputs)
        {
          if(nodeNumber.count(in))
          {
            dotFile << "n"<<nodeNumber[in] << " -> n" << nodeNumber[o] << " [label=\""<<i<<"\"];" << "\n";
          }
          i++;
        }
      }else
      if(typeid(*node) == typeid( delayedresharing::Variable))
      {
        delayedresharing::Variable* in = ( delayedresharing::Variable*) node;
          if(nodeNumber.count(in->input))
          {
            dotFile << "n"<<nodeNumber[in->input] << " -> n" << nodeNumber[in] << " [label=\"0\"];" << "\n";
          }
      }
    }
  dotFile << "}" << "\n";
  // Close the file
  dotFile.close();
}


delayedresharing::GeneralizedTerm* delayedresharing::GeneralizedTerm::clone(std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>* freshSymbolLookup)
{
                    GeneralizedTerm* clonedGeneralizedTerm = new GeneralizedTerm();
                    auto oldGeneralizedTerm = this;
                    
                    for(auto symbol : oldGeneralizedTerm->nodes)
                    {
                        clonedGeneralizedTerm->nodes.push_back(symbol->clone(freshSymbolLookup));
                    }
                    
                    
                    for(auto symbol : oldGeneralizedTerm->inputVariables)
                    {
                        clonedGeneralizedTerm->inputVariables.insert((Variable*) (*freshSymbolLookup)[symbol]);
                    }
                    
                    
                    for(auto symbol : oldGeneralizedTerm->outputVariables)
                    {
                        clonedGeneralizedTerm->outputVariables.insert((Variable*) (*freshSymbolLookup)[symbol]);
                    }

                    
                    return clonedGeneralizedTerm;
}

std::tuple<delayedresharing::GeneralizedTerm*, std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>> delayedresharing::GeneralizedTerm::clonePartial()
{
    std::map<delayedresharing::Symbol*,delayedresharing::Symbol*> freshSymbolLookup;

    for(auto inVar : this->inputVariables)
    {
        freshSymbolLookup[inVar] = inVar;
    }

    for(auto outVar : this->outputVariables)
    {
        freshSymbolLookup[outVar] = outVar;
    }
    delayedresharing::GeneralizedTerm* result = this->clone(&freshSymbolLookup);
    

    std::map<delayedresharing::Symbol*,delayedresharing::Symbol*> inputSymbolLookup;
    for(auto outVar : this->outputVariables)
    {
        inputSymbolLookup[outVar] = freshSymbolLookup[outVar->input];
    }
    return std::make_tuple(result,inputSymbolLookup);
}



void delayedresharing::GeneralizedTerm::assignDefUses()
{
  // clear defUses of all nodes but variables because a variable is used in multiple gen terms
  for(delayedresharing::Symbol* symbol: this->nodes)
  {
    if (typeid(*symbol) != typeid(delayedresharing::Variable)) {
      symbol->defUses.clear();
    }
  }

    for ( delayedresharing::Symbol *symbol : this->nodes) {
      if (typeid(*symbol) == typeid(delayedresharing::Variable)) {
        delayedresharing::Variable *v = (( delayedresharing::Variable *)symbol);
        if (v->input != nullptr) {
          v->input->defUses.insert(v);
        }
      } else if (typeid(*symbol) == typeid(delayedresharing::Operation)) {
        delayedresharing::Operation *o = (( delayedresharing::Operation *)symbol);
        for (auto input : o->inputs) {
          input->defUses.insert(o);
        }
      } else if (typeid(*symbol) == typeid(delayedresharing::Constant)) {
        // constants cannot have inputs so have no def uses elsewhere
      } else {
        std::cout << "No defUse chain to define for Symbol "
                  << typeid(*symbol).name() << "\n";
      }
    }
}

void findAll(std::set<delayedresharing::Symbol*>* symbolSet, delayedresharing::Symbol* current)
{
  std::set<delayedresharing::Symbol*> prev;
  if(typeid(*current) == typeid(delayedresharing::Variable))
  {
    delayedresharing::Variable* v = (delayedresharing::Variable*) current;
    prev.insert(v->input);
  }
  if(typeid(*current) == typeid(delayedresharing::Operation))
  {
    for(const auto& input : ((delayedresharing::Operation*) current)->inputs)
    {
      prev.insert(input);
    }
  }
  for(const auto& p : prev)
  {
    if(!symbolSet->count(p))
    {
      symbolSet->insert(p);
      if(typeid(*p) != typeid(delayedresharing::Variable))
      {
        findAll(symbolSet,p);
      }
    }
  }
}

void delayedresharing::GeneralizedTerm::updateNodeList()
{
  std::vector<delayedresharing::Symbol*> oldNodes = nodes;
  nodes.clear();
  
  std::map<delayedresharing::Symbol*,int> depthMap;
  // recursively add all nodes backwards from outputVariables
  std::set<delayedresharing::Symbol*> allSymbols;
  for(const auto& outputVar : outputVariables)
  {
    allSymbols.insert(outputVar);
    findAll(&allSymbols,outputVar);
  }

  for(const auto& symbol : allSymbols)
  {
    nodes.push_back(symbol);
  }
  // garbage collect: delete all nodes that have become unreachable
  std::set<delayedresharing::Symbol*> toDelete;
  for(const auto& oldNode : oldNodes)
  {
    if(!allSymbols.count(oldNode))
    {
      if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(oldNode))
      {
        // remove from interface
        inputVariables.erase(v);
        outputVariables.erase(v);
      }else
      {
        toDelete.insert(oldNode);
      }
    }
  }
  // 
  for(const auto& d : toDelete)
  {
    delete d;
  }
  // sort nodes list by type and topo-order
  orderNodes();
  // assign defUses
  assignDefUses();
}
     
void delayedresharing::GeneralizedTerm::reportShareAssignment()
{
        for(auto node : this->nodes)
        {
        std::cout   << node->render() << " Shares: " << node->reportShareAssignment() << "\n";
        }
}

void delayedresharing::GeneralizedTerm::orderNodes()
{
  
  std::map<delayedresharing::Symbol*,int> depthMap;
  std::stable_sort(nodes.begin(),nodes.end(),[this,&depthMap](Symbol* a, Symbol* b) {
    
    if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(a)) {
      if(this->inputVariables.count(v))
      {
        return true;
      }
      if(this->outputVariables.count(v))
      {
        return false;
      }
    }
    if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(b)) {
      if(this->inputVariables.count(v))
      {
        return false;
      }
      if(this->outputVariables.count(v))
      {
        return true;
      }
    }
    return a->SymbolDepth(&depthMap) < b->SymbolDepth(&depthMap);
  });
}



void delayedresharing::GeneralizedTerm::syntacticDeduplicate()
{
    std::map<size_t,std::vector<delayedresharing::Symbol*>> bins;
    std::map<delayedresharing::Symbol*,size_t> bin;
    // Approach: scan over each node in term and put it into bins via hash (needs to happen in topo order)
    // for var: bin = h(var_name)
    // for operation: bin = h(op->toSymbol()||bin(input_0)||bin(input_1)||...)
    for(const auto& node : nodes)
    {
      if(delayedresharing::Operation* o = dynamic_cast<delayedresharing::Operation*>(node))
      {
        size_t hash = (size_t) o->operationType;

        for(const auto& i : o->inputs)
        {
          size_t in_h;
          if(delayedresharing::Constant* c = dynamic_cast<delayedresharing::Constant*>(i))
          {
            // todo this should maybe be seperated
            in_h = c->value;
          }else
          if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(i))
          {
            in_h = reinterpret_cast<uintptr_t>(v);
          }else
          {
            in_h = bin[i];
          }
          

          hash = 31*hash + in_h;
        }
        auto b = hash;


        bin[node] = b;
        if(bins.count(b))
        {
          bins[b].push_back(node);
        }else
        {
          bins[b] = {node};
        }
      }
    }
    
    // from every bin, take exactly one node and keep it, discard remainder
    for(const auto& node : nodes){

      if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(node))
      {
        if(outputVariables.count(v))
        {
          if(bin.count(v->input))
          {
            
            auto newInput = bins[bin[v->input]][0];
            if(typeid(*newInput) != typeid(delayedresharing::Constant))
            {
            v->input = newInput;
            }
          }
        }
      }
      if(delayedresharing::Operation* o = dynamic_cast<delayedresharing::Operation*>(node))
      {
        for(int i = 0;i<o->inputs.size();i++)
        {
          if(bin.count(o->inputs[i]))
          {
            auto newInput = bins[bin[o->inputs[i]]][0];
            if(typeid(*newInput) != typeid(delayedresharing::Constant))
            {
              o->inputs[i] = newInput;
            }
          }
        }
      }
    }


    updateNodeList();
    
    assignDefUses();
    // bins do not preserve order: fix with reordering
    orderNodes();
}


void delayedresharing::GeneralizedTerm::exhaustiveDeduplicate(std::vector<std::vector<delayedresharing::Symbol*>>* bins)
{
  #ifdef DEBUG
    std::cout << "Performing exhaustive Semantic Deduplication\n";
    std::cout << "Input Variables: " << inputVariables.size() << " testing: " << (1 << (inputVariables.size())) << "\n";
  #endif
  std::vector<std::vector<delayedresharing::Symbol*>> oldBins = *bins;
  for(uint32_t value = 0;value < (1 << (inputVariables.size()));value++)
  {
    // evaluate all nodes
    std::map<delayedresharing::Symbol*,bool> values;
    for(unsigned int i = 0; i < nodes.size();i++)
    {
      delayedresharing::Symbol* s = nodes[i];
      if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(s))
      {
        if(inputVariables.count(v))
        {
          values[v] = 1 & (value >> i);
        }
      }
      if(delayedresharing::Constant* c = dynamic_cast<delayedresharing::Constant*>(s))
      {
        values[c] = c->value;
      }
      if(delayedresharing::Operation* o = dynamic_cast<delayedresharing::Operation*>(s))
      {
        if(o->operationType == delayedresharing::OperationType::ADD)
        {
          values[o] = 0;
          for(const auto& input : o->inputs)
          {
            values[o] = RingAdd<int>(values[o],values[input],2);
          }
        }else
        if(o->operationType == delayedresharing::OperationType::MULT)
        {
          values[o] = 1;
          
          for(const auto& input : o->inputs)
          {
            values[o] = RingMul<int>(values[o],values[input],2);
          }
        }
      }
    }

    // split bins if all bins of size 1 return
    std::vector<std::vector<delayedresharing::Symbol*>> freshBins;
    for(const auto& bin : oldBins)
    {
      // map value to bin
      std::map<int,std::vector<delayedresharing::Symbol*>> splitBins;
      for(const auto& binNode : bin)
      {
        if(splitBins.count(values[binNode]))
        {
          splitBins[values[binNode]].push_back(binNode);
        }else
        {
          splitBins[values[binNode]] = {binNode};
        }
      }
      for (const auto& valueBinPair : splitBins) {
        freshBins.push_back(valueBinPair.second);
      }
    }
    oldBins = freshBins;
    bool all_size_one = true;
    for(const auto& bin : oldBins)
    {
      if(bin.size() > 1)
      {
        all_size_one = false;
        break;
      }
    }
    if(all_size_one)
    {
      #ifdef DEBUG
      std::cout << "Early cancel\n";
      #endif
      return;
    }

  }

  std::map<delayedresharing::Symbol*,delayedresharing::Symbol*> binding;
  // keep only shortest symbol of bins
  for(const auto& bin : oldBins)
  {
    #ifdef DEBUG
    std::cout << "Bin of size " << bin.size() << "\n";
    #endif
    delayedresharing::Symbol* lowestSymbol = bin[0];
    std::map<delayedresharing::Symbol*,int> depthMap;
    int lowest = lowestSymbol->MultiplicativeDepth(&depthMap);
    for(int i = 1;i < bin.size();i++)
    {
      int d = bin[i]->MultiplicativeDepth(&depthMap);
      if(d<lowest)
      {
        lowest = d;
        lowestSymbol = bin[i];
        
        #ifdef DEBUG
        std::cout << "D: " << d << " " << lowestSymbol->toSymbol() << "\n";
        #endif
      }
    }
    for(const auto node : bin)
    {
      binding[node] = lowestSymbol;
    }
  }

  for(const auto& node : nodes)
  {
    if(delayedresharing::Variable* v = dynamic_cast<delayedresharing::Variable*>(node))
    {
      if(binding.count(v->input))
      {
        v->input = binding[v->input];
      }
    }
    if(delayedresharing::Operation* o = dynamic_cast<delayedresharing::Operation*>(node))
    {
      for(int i = 0;i<o->inputs.size();i++)
      {
        if(binding.count(o->inputs[i]))
        {
          o->inputs[i] = binding[o->inputs[i]];
        }
      }
    }
  }
}

void delayedresharing::GeneralizedTerm::fuzzingDeduplicate(std::vector<std::vector<delayedresharing::Symbol*>>* bins)
{
  // TODO: implement
}

void delayedresharing::GeneralizedTerm::semanticDeduplicate()
{
  #ifdef DEBUG
    std::cout << "Performing Semantic Deduplication\n";
  #endif
  int fuzzingIterations = 10;
  // if all inputs are Boolean: run exhaustively
  bool exhaustive = true;
  int exhaustive_in_var_limit = 17;
  if(inputVariables.size() > exhaustive_in_var_limit)
  {
    exhaustive = false;
  }
  for(const auto& invar : inputVariables)
  {
    if(invar->valueType.ringwidth != 1)
    {
      exhaustive = false;
    }
  }

  std::vector<std::vector<delayedresharing::Symbol*>> bins = {{}};
  for(const auto& node : nodes)
  {
    if(delayedresharing::Operation* o = dynamic_cast<delayedresharing::Operation*>(node))
    {
      bins[0].push_back(node);
    }
  }

  if(exhaustive){
    exhaustiveDeduplicate(&bins);
  }else
  {
    fuzzingDeduplicate(&bins);
  }
  
  updateNodeList();
    
  assignDefUses();
  // bins do not preserve order: fix with reordering
  orderNodes();
}

// Note: Never deduplicate constants
void delayedresharing::GeneralizedTerm::deduplicate() 
{
  syntacticDeduplicate();
  semanticDeduplicate();
}


