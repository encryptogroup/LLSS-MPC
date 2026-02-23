#include "GeneralizedTerms/GeneralizedTermNetwork.h"

void delayedresharing::Operation::assignSharings()
{
    Symbol::assignSharings();
    //allways add all sharings of input symbol
    for(auto sharing : operationSharings)
    {
        sharings.insert(std::get<1>(sharing));
    }
}

bool delayedresharing::Operation::isCommutative()
{
    return (operationType == delayedresharing::OperationType::ADD || operationType == delayedresharing::OperationType::MULT);
}

std::vector<delayedresharing::Symbol*> delayedresharing::Operation::orderedInputs()
{
    std::vector<delayedresharing::Symbol*> sortedArray;
    
    for(auto v : inputs)
    {
        int i = 0;
        while( (i<sortedArray.size()) && (reinterpret_cast<uintptr_t>(sortedArray[i]) > reinterpret_cast<uintptr_t>(v)))
        {
            i++;
        }
        sortedArray.insert(sortedArray.begin() + i, v); 
    }
    return sortedArray;
}

std::string delayedresharing::Operation::render()
{
    if(operationType == delayedresharing::OperationType::INPUT)
    {
        return toSymbol();
    }
    if(operationType == delayedresharing::OperationType::OUTPUT)
    {
        return toSymbol();
    }
    std::string output = toSymbol()+"(";
    for(int i = 0;i<inputs.size();i++)
    {
        output +=inputs[i]->render();
        if(i<(inputs.size()-1))
        {
            output += ",";
        }
    }
    return output+")";
}

delayedresharing::Operation* findDefiningOperation(delayedresharing::Symbol* inputSymbol)
{
    
    if(delayedresharing::Operation* inputOp = dynamic_cast<delayedresharing::Operation*>(inputSymbol)) {
        // old was safely casted to NewType
        return inputOp;
    }else if(delayedresharing::Variable* inputVar = dynamic_cast<delayedresharing::Variable*>(inputSymbol)) {
        // old was safely casted to NewType
        return findDefiningOperation(inputVar->input);
    }else if(delayedresharing::Constant* inputVar = dynamic_cast<delayedresharing::Constant*>(inputSymbol)) {
        // old was safely casted to NewType
        return nullptr;
    }

    return nullptr;
}

std::vector<delayedresharing::Operation*> delayedresharing::Operation::findInputOperations()
{
                std::vector< delayedresharing::Operation*> result;

                for(auto input : this->inputs)
                {
                    delayedresharing::Operation* opOfInput = findDefiningOperation(input);
                    if(opOfInput != nullptr)
                    {
                        result.push_back(opOfInput);
                    }
                }
                return result;
}

std::vector<delayedresharing::Operation*> delayedresharing::Operation::findNextOperations()
{
    std::vector<delayedresharing::Operation*> result;

    for(auto use : this->defUses)
    {
        if(typeid(*use) == typeid( delayedresharing::Operation))
        {
            delayedresharing::Operation* o = ( delayedresharing::Operation*) use;
            result.push_back(o);
        } else if(typeid(*use) == typeid( delayedresharing::Variable))
        {
            for(auto next_use : use->defUses)
            {
                // no variable can follow another variable
                if(typeid(*next_use) == typeid( delayedresharing::Operation))
                {
                    delayedresharing::Operation* on = ( delayedresharing::Operation*) next_use;
                    result.push_back(on);
                }
            }
        }
    }
    return result;
}



int delayedresharing::Operation::SymbolDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
{
    if((*depthMap).count(this))
    {
        return (*depthMap)[this];
    }else
    {
        int max = 0;
        for(auto input : inputs)
        {
            int c = input->SymbolDepth(depthMap);
            if(c>max)
            {
                max = c;
            }
        }
        int result = 1+max;
        (*depthMap)[this] = result;
        

        return result;
    }
}
int delayedresharing::Operation::MultiplicativeDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
{
    if((*depthMap).count(this))
    {
        return (*depthMap)[this];
    }else
    {
        int max = 0;
        for(auto input : inputs)
        {
            int c = input->MultiplicativeDepth(depthMap);
            if(c>max)
            {
                max = c;
            }
        }
        int offset = 0;
        switch (operationType)
        {
        case delayedresharing::OperationType::MULT:
            offset = 1;
            break;
        case delayedresharing::OperationType::DOTPRODUCT:
            offset = 1;
            break;
        default:
            break;
        }
        int result = offset+max;
        (*depthMap)[this] = result;

        return result;
    }
}
