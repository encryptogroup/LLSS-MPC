#include <GeneralizedTerms/GeneralizedTermNetwork.h>
#include <Optimizations/ShareAssignment/ShareAssignment.h>
#include <GeneralizedTerms/Definitions.h>

/*
    This is just a helper for benchmarking
    in reality we would like to build this from a RTL like IR
    Circuits are read into a blank GeneralizedTermNetwork without any sharing
   annotation and fully binary in operations The inputs, each operation and the
   outputs form a Generalized Term each
*/

 delayedresharing::GeneralizedTermNetwork* delayedresharing::GeneralizedTermNetwork::fromBristol(std::string path) {
  std::cout << "Loading Bristol Generalized Term Network from " << path << "\n";
  if (!std::filesystem::exists(path)) {
    std::cout << "file " << path << " does not exist" << std::endl;
    std::exit(1);
  }
  delayedresharing::GeneralizedTermNetwork *network = new delayedresharing::GeneralizedTermNetwork();
  std::ifstream bristolFile;
  bristolFile.open(path);
  std::string line;
  std::vector< delayedresharing::Variable *> variables;


  // create variables
  std::getline(bristolFile, line);

  std::stringstream ss(line);
  std::string inputs_token;

  std::getline(ss, inputs_token, ' ');
  std::getline(ss, inputs_token, ' ');
  

  ValueType valueType;
  valueType.ringwidth = 1;
  for (int i = 0; i < std::stoi(inputs_token); i++) {

    delayedresharing::Variable *variable = new delayedresharing::Variable();
    variable->variableName = std::to_string(i);
    variable->input = nullptr;
    variable->valueType = valueType;
    variables.push_back(variable);
  }

  std::string token;

  // process output numbers
  // process input numbers

  std::getline(bristolFile, line);
  int in_a = 0;
  int in_b = 0;
  int out = 0;
  std::stringstream ns(line);
  std::getline(ns, token, ' ');
  in_a = std::stoi(token);
  std::getline(ns, token, ' ');
  in_b = std::stoi(token);
  std::getline(ns, token, ' ');
  out = std::stoi(token);

  delayedresharing::GeneralizedTerm *inputGeneralizedTerm = new delayedresharing::GeneralizedTerm();
  for (int i = 0; i < in_a; i++) {
    delayedresharing::Operation *o = new delayedresharing::Operation();
    o->operationType = delayedresharing::OperationType::INPUT;
    o->valueType = valueType;
    o->signature = {valueType,valueType};
    delayedresharing::Variable *v = variables[i];
    inputGeneralizedTerm->nodes.push_back(o);
    v->input = o;
    v->valueType = valueType;
    inputGeneralizedTerm->outputVariables.insert(v);
  }
  for (int i = 0; i < in_b; i++) {
    delayedresharing::Operation *o = new delayedresharing::Operation();
    o->operationType = delayedresharing::OperationType::INPUT;
    o->valueType = valueType;
    o->signature ={valueType,valueType};
    delayedresharing::Variable *v = variables[in_a + i];
    inputGeneralizedTerm->nodes.push_back(o);
    v->input = o;
    v->valueType = valueType;
    inputGeneralizedTerm->outputVariables.insert(v);
  }

  network->generalizedTerms.push_back(inputGeneralizedTerm);
  network->inputs = inputGeneralizedTerm;

  // TODO: maybe do not inputs these two generalized terms into general set

  std::getline(bristolFile, line);

  // create input variables

  std::vector<std::string> lines;
  while (bristolFile) {
    std::getline(bristolFile, line);
    lines.push_back(line);
    if (bristolFile.eof()) {
      break;
    }
  }

  for (int i = 0; i < lines.size(); i++) {
    line = lines[i];
    if(line == "")
    {
      continue;
    }
    std::stringstream ss(line);
    std::string token;
    std::getline(ss, token, ' ');
    int num_in = std::stoi(token);
    std::getline(ss, token, ' ');
    int num_out = std::stoi(token);

    int inputWires[2];

    delayedresharing::GeneralizedTerm *newGeneralizedTerm = new delayedresharing::GeneralizedTerm();

    std::vector<delayedresharing::Variable*> opInputs;
    delayedresharing::Variable* outVar;
    for (int j = 0; j < num_in; j++) {
      std::getline(ss, token, ' ');
      int wireNumber = std::stoi(token);
      inputWires[j] = wireNumber;
      opInputs.push_back(variables[wireNumber]);
      newGeneralizedTerm->inputVariables.insert(variables[wireNumber]);
    }


    for (int j = 0; j < num_out; j++) {
      std::getline(ss, token, ' ');
      int wireNumber = std::stoi(token);
      outVar = variables[wireNumber];
      outVar->valueType = valueType;
      newGeneralizedTerm->outputVariables.insert(variables[wireNumber]);
    }
    
    delayedresharing::Operation *newOperator = new delayedresharing::Operation();
    newOperator->valueType = valueType;
    newOperator->signature = {valueType,valueType};
    std::getline(ss, token, ' ');

    

    if (token == "AND") {
      newOperator->operationType = delayedresharing::OperationType::MULT;
      newGeneralizedTerm->nodes.push_back(newOperator);
      for(auto input : opInputs)
      {
        newOperator->inputs.push_back(input);
      }
      outVar->input = newOperator;
      outVar->valueType = valueType;
    } else if (token == "XOR") {
      newOperator->operationType = delayedresharing::OperationType::ADD;
      newOperator->valueType = valueType;
      newGeneralizedTerm->nodes.push_back(newOperator);
      for(auto input : opInputs)
      {
        newOperator->inputs.push_back(input);
      }
      outVar->input = newOperator;
      outVar->valueType = valueType;
    } else if (token == "OR") {
      
      for(auto input : opInputs)
      {
        
        delayedresharing::Constant* one = new delayedresharing::Constant();
        one->value = 1;
        one->valueType = valueType;
        newGeneralizedTerm->nodes.push_back(one);
        delayedresharing::Operation* negation = new delayedresharing::Operation();
        negation->inputs.push_back(one);
        negation->inputs.push_back(input);
        negation->operationType = delayedresharing::OperationType::ADD;
        negation->valueType = valueType;
        negation->signature = {valueType,valueType};
        newOperator->inputs.push_back(negation);
        newGeneralizedTerm->nodes.push_back(negation);
      }
      newOperator->operationType = delayedresharing::OperationType::MULT;
      newOperator->valueType = valueType;
      newOperator->signature = {valueType,valueType};
      
      delayedresharing::Operation* negation = new delayedresharing::Operation();
      negation->operationType = delayedresharing::OperationType::ADD;
      negation->valueType = valueType;
      negation->signature = {valueType,valueType};
      
      delayedresharing::Constant* one = new delayedresharing::Constant();
      one->valueType = valueType;
      one->value = 1;
      negation->inputs.push_back(one);
      negation->inputs.push_back(newOperator);
      
      newGeneralizedTerm->nodes.push_back(newOperator);
      newGeneralizedTerm->nodes.push_back(one);
      newGeneralizedTerm->nodes.push_back(negation);

      outVar->input = negation;
    } else if (token == "INV") {
      newOperator->operationType = delayedresharing::OperationType::ADD;
      newOperator->valueType = valueType;
      for(auto input : opInputs)
      {
        newOperator->inputs.push_back(input);
      }
      
      delayedresharing::Constant *one = new delayedresharing::Constant();
      one->valueType = valueType;
      one->value = 1;
      newOperator->inputs.push_back(one);
      newGeneralizedTerm->nodes.push_back(one);
      newGeneralizedTerm->nodes.push_back(newOperator);
      outVar->input = newOperator;
      outVar->valueType = valueType;
    } else {
      std::cout << "Could not parse '" << token << "'" << "\n";
    }
    network->generalizedTerms.push_back(newGeneralizedTerm);
  }

  delayedresharing::GeneralizedTerm *outputGeneralizedTerm = new delayedresharing::GeneralizedTerm();
  for (int i = 0; i < out; i++) {
    delayedresharing::Operation *o = new delayedresharing::Operation();
    o->operationType = delayedresharing::OperationType::OUTPUT;
    o->valueType = valueType;
    o->signature = {valueType,valueType};
    delayedresharing::Variable *v = variables[variables.size() - out + i];
    v->valueType = valueType;
    outputGeneralizedTerm->nodes.push_back(o);
    o->inputs.push_back(v);
    outputGeneralizedTerm->inputVariables.insert(v);
  }
  network->generalizedTerms.push_back(outputGeneralizedTerm);
  network->outputs = outputGeneralizedTerm;

  for(delayedresharing::Variable* var : variables)
  {
    assert(var->input != nullptr);
  }
  
  network->addMissingVariables();

  network->assignDefUses();

  // assign sharing as plain sharing with ringwidth 1 because we operate with Boolean computation

  delayedresharing::PlainShareAssignment* plainAssignment = new delayedresharing::PlainShareAssignment();
  plainAssignment->apply(network);

  delete plainAssignment;
  std::cout << "Loading finished" << "\n";
  return network;
}


void delayedresharing::GeneralizedTermNetwork::toDot(std::string path) {

  // Create and open a text file
  std::ofstream dotFile(path);

  // Write to the file
  dotFile << "digraph G {" << "\n";

  int termCounter = 0;
  int nodeCounter = 0;
  std::map< delayedresharing::Symbol*,int> nodeNumber;
  for(auto genTerm : generalizedTerms)
  {
    dotFile << "    subgraph cluster_" << termCounter << " {" << "\n";
    dotFile << "        style=filled;" << "\n";
    dotFile << "        color=grey;" << "\n";

    for(auto node : genTerm->nodes)
    {
        if(!nodeNumber.count(node))
        {
            nodeNumber[node] = nodeCounter;
            dotFile << "        n"<<nodeCounter<< delayedresharing::NodeText(node) <<"\n";
            nodeCounter++;
        }
    }

    dotFile << "        label = \"generalized term "<<termCounter<<"\"" << "\n";
    dotFile << "    }" << "\n" << "\n";
    termCounter++;
  }

  // render all edges
  std::set< delayedresharing::Symbol*> symbols;
  for(auto genTerm : generalizedTerms)
  {
    for(auto node : genTerm->nodes)
    {
    symbols.insert(node);
    }
  }
    for(auto node : symbols)
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
          }else{
            std::cout << "Node " << in << " does not exist in delayedresharingN\n";
          }
          i++;
        }
      }else
      if(typeid(*node) == typeid( delayedresharing::Variable))
      {
        delayedresharing::Variable* in = ( delayedresharing::Variable*) node;
          if(nodeNumber.count(in->input))
          {
            dotFile << "n"<<nodeNumber[in->input] << " -> n" << nodeNumber[in] << " [label=\"1\"];" << "\n";
          }else{
            std::cout << "Node " << in->input << " does not exist in delayedresharingN\n";
          }
      }
    }
  dotFile << "}" << "\n";
  // Close the file
  dotFile.flush();
  dotFile.close();
}


void delayedresharing::ComputationHyperGraph::toDot(std::string path) {

  // Create and open a text file
  std::ofstream dotFile(path);

  // Write to the file
  dotFile << "digraph G {" << "\n";
  for(int i = 0;i<num_nodes;i++)
  {
    dotFile << "n" << i;
    dotFile << "[xlabel=\"" << index[i]->toSymbol();
    if(partition.size() > i)
    {
      dotFile << partition[i];
    }
    dotFile << "\"]";
    dotFile << ";";
  }
  for(int i = 0;i<edges.size();i++)
  {
    auto edge = edges[i];
    for(int j = 1;j<edge->members.size();j++)
    {
        dotFile << "n"<<edge->members[0] << " -> n"<<edge->members[j] << " [label=\"Edge "<<j<<"\"];" << "\n";
    }
  }
  dotFile << "}" << "\n";
  // Close the file
  dotFile.flush();
  dotFile.close();
}



void delayedresharing::GeneralizedTermNetwork::toFile(std::string path)
{
    // Create and open a text file
  std::ofstream outputFile(path);

  // Write to the file
  std::map<delayedresharing::Operation*,int> wireNumber;
  int wireCounter = 0;

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



  for( delayedresharing::Operation* o : operations)
  {
    wireNumber[o] = wireCounter;
    wireCounter++;
  }

  for(delayedresharing::Operation* o : operations)
  {
        outputFile << o->toSymbol() << ":" << ValueTypeName(std::get<0>(o->signature)) << ">" << ValueTypeName(std::get<1>(o->signature)) << " < ";

        int i = 0;
        for(auto conversion : o->conversions)
        {
          auto a = std::get<0>(conversion);
          auto b = std::get<1>(conversion);
          outputFile << SharingModeName(a) << ":" << SharingModeName(b);
          outputFile << " ";
          i++;
        }
        
        outputFile << "> { ";

        i = 0;
        for(auto operationSharing : o->operationSharings)
        {
          auto a = std::get<0>(operationSharing);
          auto b = std::get<1>(operationSharing);
          outputFile << SharingModeName(a) << ":" << SharingModeName(b);
          outputFile << " ";
          i++;
        }

        outputFile << "} ";

        for(auto input : o->inputs)
        {
          if(typeid(*input) == typeid(delayedresharing::Constant))
          {
            delayedresharing::Constant* c = (delayedresharing::Constant*) input;
            outputFile << "c" << c->value << " ";
          }
        }

        for(auto input : o->findInputOperations())
        {
          outputFile << wireNumber[input] << " ";
        }
        outputFile << "| " << wireNumber[o] << "\n";
  }
  // Close the file
  outputFile.flush();
  outputFile.close();
}






delayedresharing::GeneralizedTermNetwork* delayedresharing::GeneralizedTermNetwork::fromFile(std::string path)
{
  std::cout << "Loading Generalized Term Network from " << path << "\n";
  if (!std::filesystem::exists(path)) {
    std::cout << "file " << path << " does not exist" << std::endl;
    std::exit(1);
  }
  delayedresharing::GeneralizedTermNetwork *network = new delayedresharing::GeneralizedTermNetwork();
  std::ifstream inputFile;
  inputFile.open(path);

  delayedresharing::GeneralizedTerm* inputTerm = new delayedresharing::GeneralizedTerm();
  delayedresharing::GeneralizedTerm* computationTerm = new delayedresharing::GeneralizedTerm();
  delayedresharing::GeneralizedTerm* outputTerm = new delayedresharing::GeneralizedTerm();
  network->inputs = inputTerm;
  network->outputs = outputTerm;
  network->generalizedTerms.push_back(inputTerm);
  network->generalizedTerms.push_back(computationTerm);
  network->generalizedTerms.push_back(outputTerm);

  std::map<int,delayedresharing::Symbol*> wireConnection;

  int variableCounter = 1;


  if(inputFile.is_open()){
        while(!inputFile.eof()){
            std::string line;
            getline(inputFile,line);
            if(line == "")
            {
              break;
            }
            
            std::vector<std::string> tokens = split(line, ' ');
            
            std::vector<std::string> opCode = split(tokens[0],':');

            delayedresharing::Operation* operation = new delayedresharing::Operation();
            delayedresharing::OperationType operationType = OperationTypeFromString(opCode[0]);
            operation->operationType = operationType;
            
            std::vector<std::string> valueTypes = split(opCode[1],'>');
            ValueType valueTypeA = ValueTypeFromString(valueTypes[0]);
            ValueType valueTypeB = ValueTypeFromString(valueTypes[1]);

            operation->valueType = valueTypeB;
            operation->signature = {valueTypeA,valueTypeB};

            int i = 1;
            while(tokens[i] != ">")
            {
              i++;
            }

            std::vector<std::string> conversions(tokens.begin()+2,tokens.begin()+i);

            i++;
            int k = i;
            
            while(tokens[i] != "}")
            {
              i++;
            }

            std::vector<std::string> operationSharings(tokens.begin()+k+1,tokens.begin()+i);

            int l = i + 1;
            while(tokens[i] != "|")
            {
              i++;
            }

            std::vector<std::string> inputs(tokens.begin()+l,tokens.begin()+i);

            std::string output = tokens[i+1];

            for(auto conversion : conversions)
            {
              auto conversionTuple = delayedresharing::SharingTupleFromString(conversion);
              operation->conversions.insert(conversionTuple);
            }

            for(auto operationSharing : operationSharings)
            {
              auto operationSharingTuple = delayedresharing::SharingTupleFromString(operationSharing);
              operation->operationSharings.insert(operationSharingTuple);
            }

            switch(operationType)
            {
              case delayedresharing::OperationType::INPUT:
              {
                inputTerm->nodes.push_back(operation);
                delayedresharing::Variable* v = new delayedresharing::Variable();
                v->valueType = valueTypeB;
                v->input = operation;
                v->variableName = std::to_string(variableCounter);
                variableCounter++;
                inputTerm->outputVariables.insert(v);
                computationTerm->inputVariables.insert(v);
                wireConnection[std::stoi(output)] = v;
                break;
              }
              case delayedresharing::OperationType::ADD:
              {
                for(auto input : inputs)
                {
                  if(input.starts_with('c'))
                  {
                    delayedresharing::Constant* constant = new delayedresharing::Constant();
                    constant->valueType = valueTypeA;
                    operation->inputs.push_back(constant);
                    std::string valueString = input.substr(1);
                    constant->value = std::stoi(valueString);
                    computationTerm->nodes.push_back(constant);
                  }else
                  {
                    operation->inputs.push_back(wireConnection[std::stoi(input)]);
                  }
                }
                computationTerm->nodes.push_back(operation);
                wireConnection[std::stoi(output)] = operation;
                break;
              }
              case delayedresharing::OperationType::MULT:
              {
                for(auto input : inputs)
                {
                  if(input.starts_with('c'))
                  {
                    delayedresharing::Constant* constant = new delayedresharing::Constant();
                    constant->valueType = valueTypeA;
                    operation->inputs.push_back(constant);
                    std::string valueString = input.substr(1);
                    constant->value = std::stoi(valueString);
                    computationTerm->nodes.push_back(constant);
                  }else
                  {
                    operation->inputs.push_back(wireConnection[std::stoi(input)]);
                  }
                }
                computationTerm->nodes.push_back(operation);
                wireConnection[std::stoi(output)] = operation;

                break;
              }
              case delayedresharing::OperationType::OUTPUT:
              {
                outputTerm->nodes.push_back(operation);
                delayedresharing::Variable* v = new delayedresharing::Variable();
                v->valueType = valueTypeA;
                v->input = wireConnection[std::stoi(inputs[0])];

                v->variableName = std::to_string(variableCounter);
                variableCounter++;
                outputTerm->inputVariables.insert(v);
                computationTerm->outputVariables.insert(v);
                operation->inputs.push_back(v);
                break;
              }
            }
        }
        inputFile.close();
  }
  network->addMissingVariables();

  network->assignDefUses();

  network->assignSharings();

  return network;
}


std::string delayedresharing::GeneralizedTerm::toString()
            {
                std::map<delayedresharing::Symbol*,int> depthMap;
                auto terms = Terms();
                std::string output = "GenTerm {\n";
                output += "InputVariables: (\n";
                
                int k = 0;
                for(auto i : inputVariables)
                {
                    output+= i->toSymbol();
                    if(k<(inputVariables.size()-1))
                    {
                      output += " ,";
                    }
                    k++;
                }
                output += ")\n";
                output += "OutputVariables: (\n";
                k = 0;
                for(auto o : outputVariables)
                {
                  output+= o->toSymbol();
                  
                  if(k<(outputVariables.size()-1))
                  {
                    output += " ,";
                  }
                  k++;
                }
                output += ")\n";

                output+= "Nodes: (\n";
                for(int i = 0;i<nodes.size();i++)
                {
                    output+= nodes[i]->toSymbol()+" " + typeid(nodes[i]).name() + " : "+ pointer_to_string(nodes[i])+" Uses: "+std::to_string((int)nodes[i]->defUses.size())+" Depth: "+ std::to_string(nodes[i]->SymbolDepth(&depthMap));
                    if(i<(nodes.size()-1))
                    {
                    output+=",\n";
                    }
                }
                output += ")\n";
                output += "Terms:\n";
                for(int i = 0;i<terms.size();i++)
                {
                    output += terms[i];
                    if(i<(terms.size()-1))
                    {
                        output += ",\n";
                    }
                }
                output += "\n}";
                return output;
            }

// note this format requires topological order, and that all variables be ordered according to generalized term standard: in_vars > ops > out_vars
// this loading is used to load rules: for now rules have no value types
delayedresharing::GeneralizedTerm* delayedresharing::GeneralizedTerm::fromFile(std::string path)
{
  if (!std::filesystem::exists(path)) {
    std::cout << "file " << path << " does not exist" << std::endl;
    std::exit(1);
  }
  delayedresharing::GeneralizedTerm* genTerm = new delayedresharing::GeneralizedTerm();
  std::ifstream inputFile;
  
  inputFile.open(path);
  delayedresharing::ValueType val;
  val.ringwidth = 1;
  std::vector<delayedresharing::Symbol*> symbolLookup;

  if(inputFile.is_open()){
      int c = 0;
      while(!inputFile.eof()){
            std::string line;
            getline(inputFile,line);
            if(line == "")
            {
              break;
            }
            std::vector<std::string> tokens = split(line, ' ');
            if(tokens[0].starts_with("v"))
            {
              delayedresharing::Variable* v = new delayedresharing::Variable();
              v->variableName = tokens[0].substr(1,tokens[0].length()-1);
              v->valueType = val;
              for(int i = 1;i<tokens.size();i++)
              {
                v->input = symbolLookup[std::stoi(tokens[i])];
              }
              if(tokens.size() > 1)
              {
                genTerm->outputVariables.insert(v);
              }else
              {
                genTerm->inputVariables.insert(v);
              }
              genTerm->nodes.push_back(v);
              symbolLookup.push_back(v);
            }else
            {
              delayedresharing::Operation* o = new delayedresharing::Operation();
              o->valueType = val;
              o->signature = {val,val};
              o->operationType = OperationTypeFromString(tokens[0]);
              for(int i = 1;i<tokens.size();i++)
              {
                if(tokens[i].starts_with("c"))
                {
                  // todo constants
                  delayedresharing::Constant* c = new delayedresharing::Constant();
                  c->valueType = val;
                  c->value = std::stoi(tokens[i].substr(1,tokens[i].length()-1));
                  o->inputs.push_back(c);
                  genTerm->nodes.push_back(c);
                }else
                {
                  o->inputs.push_back(symbolLookup[std::stoi(tokens[i])]);
                }
              }
              genTerm->nodes.push_back(o);
              
              symbolLookup.push_back(o);
            }
      }
    }

  inputFile.close();

  genTerm->assignDefUses();
  return genTerm;
}














