#include "Partition.h"
#include <queue>
#include <fstream>




delayedresharing::GeneralizedTermNetwork* delayedresharing::Partition::applyFresh( delayedresharing::GeneralizedTermNetwork *termNetwork) 
{
    std::cout << "Partitioning into " << k << " Groups" << "\n";

    
    delayedresharing::ComputationHyperGraph* computationHyperGraph = termNetwork->extractComputationGraph();
    
    std::vector<int> partition(computationHyperGraph->num_nodes, 0);
    // in the case of k = 1 everything is in one partition: do not call partition algorithm as optimization
    for(int i = 0;i<partition.size();i++)
    {
        partition[i] = 0;
    }
    

    for(int i = 0;i<partition.size();i++)
    {
      partition[i] = partition[i] + 1;
    }
    


    delayedresharing::GeneralizedTermNetwork *partitionedTermNetwork = new delayedresharing::GeneralizedTermNetwork();

    int variableCounter = 0;
    // This tells us which variable an outputting operation of a GT is assigned
    std::map< delayedresharing::Operation*, delayedresharing::Variable *> freshVariables;

    // maps new Operation -> old Operation
    std::map< delayedresharing::Operation*, delayedresharing::Operation*> oldOperation;

    // maps oldOperation -> new Operation
    std::map< delayedresharing::Operation*, delayedresharing::Operation*> newOperation;


    std::map<int,delayedresharing::GeneralizedTerm*> generalizedTermOfPartition;

    // maps symbol to its generalized term
    std::map<delayedresharing::Symbol*,delayedresharing::GeneralizedTerm*> generalizedTermOfSymbol;


    // 1: copy all operations

    // Create Input GT with its operation
    delayedresharing::GeneralizedTerm *inputTerm = new delayedresharing::GeneralizedTerm();
    for (auto node : termNetwork->inputs->nodes) {
      if (typeid(*node) == typeid( delayedresharing::Operation)) {
        delayedresharing::Operation *oldInputOp = (Operation*) node;
        delayedresharing::Operation *newInputOp = new delayedresharing::Operation();

        generalizedTermOfSymbol[oldInputOp] = inputTerm;
        generalizedTermOfSymbol[newInputOp] = inputTerm;

        newInputOp->valueType = oldInputOp->valueType;
        newInputOp->signature = oldInputOp->signature;
        newInputOp->operationType = delayedresharing::OperationType::INPUT;
        inputTerm->nodes.push_back(newInputOp);
        newOperation[oldInputOp] = newInputOp;
        oldOperation[newInputOp] = oldInputOp;
      }
    }
    partitionedTermNetwork->generalizedTerms.push_back(inputTerm);
    partitionedTermNetwork->inputs = inputTerm;
    generalizedTermOfPartition[0] = inputTerm;


    // collapse all  computing operations into partitioning map
    int createdIntermediates = 0;
    for(int i = 0; i < computationHyperGraph->num_nodes;i++)
    {
      int partition_of_node = partition[i];
      delayedresharing::Operation* oldOp = computationHyperGraph->index[i];

      delayedresharing::GeneralizedTerm* genTermOfOp;
      if(generalizedTermOfPartition.count(partition_of_node) == 0)
      {
        genTermOfOp = new delayedresharing::GeneralizedTerm();
        partitionedTermNetwork->generalizedTerms.push_back(genTermOfOp);
        generalizedTermOfPartition[partition_of_node] = genTermOfOp;
        createdIntermediates++;
      }else
      {
        genTermOfOp = generalizedTermOfPartition[partition_of_node];
      }
      generalizedTermOfSymbol[oldOp] = genTermOfOp;
      delayedresharing::Operation* newOp = new delayedresharing::Operation();
      generalizedTermOfSymbol[newOp] = genTermOfOp;
      newOp->operationType = oldOp->operationType;
      newOp->valueType = oldOp->valueType;
      newOp->signature = oldOp->signature;
      // TODO: add constant inputs here if needed
      for(auto in : oldOp->inputs)
      {
        if(typeid(*in) == typeid(delayedresharing::Constant))
        {
          delayedresharing::Constant* cin = (Constant*) in;
          delayedresharing::Constant* newConstant = new delayedresharing::Constant();
          newConstant->value = cin->value;
          newConstant->valueType = cin->valueType;
          newOp->inputs.push_back(newConstant);
          generalizedTermOfSymbol[newOp]->nodes.push_back(newConstant);
        }
      }

      oldOperation[newOp] = oldOp;
      newOperation[oldOp] = newOp;
      
      genTermOfOp->nodes.push_back(newOp);
    }


    // Create Output GT with its operation

    delayedresharing::GeneralizedTerm* outputTerm = new delayedresharing::GeneralizedTerm();
    
    for (auto node : termNetwork->outputs->nodes) {

      if (typeid(*node) == typeid( delayedresharing::Operation)) {
        delayedresharing::Operation *newOutputOp = new delayedresharing::Operation();
        delayedresharing::Operation *oldOutputOp = (Operation*) node;
        generalizedTermOfSymbol[newOutputOp] = outputTerm;
        generalizedTermOfSymbol[oldOutputOp] = outputTerm;
        newOutputOp->operationType = delayedresharing::OperationType::OUTPUT;
        newOutputOp->valueType = oldOutputOp->valueType;
        newOutputOp->signature = oldOutputOp->signature;
        outputTerm->nodes.push_back(newOutputOp);
        newOperation[oldOutputOp] = newOutputOp;
        oldOperation[newOutputOp] =  oldOutputOp;
      }
    }
    partitionedTermNetwork->generalizedTerms.push_back(outputTerm);
    partitionedTermNetwork->outputs = outputTerm;


    // 2: connect operations: if it crosses boundary, create variable for starting operation and add receiving to it, else directly add to input
    
    // note: this should be in topological order but it does not necessarily matter

    // second note: currently this does not respect order of inputs, as constants could be out of order, this should be fixed at some point
    for(const auto& pair : oldOperation)
    {
      delayedresharing::Operation* newOp = pair.first;
      delayedresharing::Operation* oldOp = pair.second;
      auto inputsOfOldOperation = oldOp->findInputOperations();
      for(auto oldInput : inputsOfOldOperation)
      {
        delayedresharing::Operation* newInput = newOperation[oldInput];

        if(generalizedTermOfSymbol[oldInput] != generalizedTermOfSymbol[oldOp])
        {
          // create fresh output Variable for newInput

          delayedresharing::Variable* operationOutputVariable;
          if(freshVariables.count(newInput) == 0)
          {
            operationOutputVariable = new delayedresharing::Variable();
            operationOutputVariable->input = newInput;
            operationOutputVariable->valueType = newInput->valueType;
            operationOutputVariable->variableName = std::__cxx11::to_string(variableCounter);
            freshVariables[newInput] = operationOutputVariable;
            variableCounter++;
          }else
          {
            operationOutputVariable = freshVariables[newInput];
          }

          generalizedTermOfSymbol[newInput]->outputVariables.insert(operationOutputVariable);
          generalizedTermOfSymbol[newOp]->inputVariables.insert(operationOutputVariable);

          newOp->inputs.push_back(operationOutputVariable);
        }else
        {
          newOp->inputs.push_back(newInput);
        }
      }
    }


    delete computationHyperGraph;
    

    // insert input and output Variables into their generalized terms node lists
    partitionedTermNetwork->addMissingVariables();

    partitionedTermNetwork->assignDefUses();

    for(int i = 1;i<partitionedTermNetwork->generalizedTerms.size()-1;i++)
    {
      partitionedTermNetwork->generalizedTerms[i]->orderNodes();
    }

    #ifdef DEBUG
    for(int i = 0;i<partitionedTermNetwork->generalizedTerms.size();i++)
    {
      delayedresharing::GeneralizedTerm* term = partitionedTermNetwork->generalizedTerms[i];
      term->toDot("dotFiles/"+std::to_string(i)+".dot");
      std::cout << "TermStats for Term "<< std::to_string(i)<<":\n";
      std::cout << "InVars: " << term->inputVariables.size() << "\n";
      std::cout << "OutVars: " << term->outputVariables.size() << "\n";
    }
    #endif

    std::cout << "Finished Partition" << "\n";
    return partitionedTermNetwork;
  }