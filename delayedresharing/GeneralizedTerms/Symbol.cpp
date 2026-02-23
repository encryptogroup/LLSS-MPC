#include "GeneralizedTerms/GeneralizedTermNetwork.h"

std::vector<delayedresharing::Symbol*> delayedresharing::Symbol::InputSymbols()
{
                std::vector<delayedresharing::Symbol*> results;
                if(typeid(*this) == typeid(delayedresharing::Variable))
                {
                    results.push_back(((delayedresharing::Variable*)this)->input);
                }else if(typeid(*this) == typeid(Operation))
                {
                    delayedresharing::Operation* o = (delayedresharing::Operation*) this;
                    results.insert(results.end(),o->inputs.begin(),o->inputs.end());
                }
                return results;
}



std::string delayedresharing::Symbol::render()
{
    return toSymbol();
}

std::string delayedresharing::Symbol::toSymbol()
{
    return "?";
}



std::string delayedresharing::Symbol::reportShareAssignment()
{
                std::string output = "";
                if(sharings.size() > 0)
                {
                output = "Sharings: (";
                int i = 0;
                for(auto sharing : sharings)
                {
                    output += delayedresharing::SharingModeName(sharing);
                    if(i < (sharings.size()-1))
                        output +=",";
                    i++;
                }
                output += ")";
                }
                if(conversions.size() > 0)
                {
                output += " Conversions: (";
                int i = 0;
                for(auto conversion : conversions)
                {
                    auto a = std::get<0>(conversion);
                    auto b = std::get<1>(conversion);
                    output += delayedresharing::SharingModeName(a)+"->"+ delayedresharing::SharingModeName(b);
                    if(i < (conversions.size()-1))
                        output += ",";
                    i++;
                }
                output += ")";
                }
                return output;
}

delayedresharing::Symbol* delayedresharing::Symbol::clone(std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>* freshSymbolLookup)
{
    if((*freshSymbolLookup).count(this))
    {
        return (*freshSymbolLookup)[this];
    }else{
        delayedresharing::Symbol* freshSymbol;

                    if(typeid(*this) == typeid(delayedresharing::Constant))
                    {
                        freshSymbol = new delayedresharing::Constant();
                        freshSymbol->valueType = this->valueType;

                       ((delayedresharing::Constant*) freshSymbol)->value = ((delayedresharing::Constant*) this)->value;
                    }else
                    if(typeid(*this) == typeid(delayedresharing::Variable))
                    {
                        freshSymbol = new delayedresharing::Variable();
                        freshSymbol->valueType = this->valueType;
                        ((Variable*) freshSymbol)->variableName = ((Variable*) this)->variableName;
                        ((Variable*) freshSymbol)->input = (*freshSymbolLookup)[ ((Variable*) this)->input ];
                    }else
                    if(typeid(*this) == typeid(delayedresharing::Operation))
                    {
                        freshSymbol = new delayedresharing::Operation();
                        auto oldOp = ((delayedresharing::Operation*) this);
                        auto freshOp = ((delayedresharing::Operation*) freshSymbol);
                        freshOp->operationType = oldOp->operationType;
                        freshOp->signature = oldOp->signature;
                        freshSymbol->valueType = this->valueType;
                        for(auto input : oldOp->inputs)
                        {
                            freshOp->inputs.push_back((*freshSymbolLookup)[input]);
                        }
                    }else{
                        // could not clone symbol
                        assert(false);
                    }
                    
        freshSymbol->valueType = this->valueType;
        (*freshSymbolLookup)[this] = freshSymbol;
        return freshSymbol;
    }
  }


void delayedresharing::Symbol::assignSharings()
{
    sharings.clear();
    for(auto conversion : conversions)
    {
        sharings.insert(std::get<1>(conversion));
    }
}

void delayedresharing::Variable::assignSharings()
{
    Symbol::assignSharings();
    //allways add all sharings of input symbol
    for(auto sharing : input->sharings)
    {
        sharings.insert(sharing);
    }
}


void delayedresharing::Constant::assignSharings()
{
    Symbol::assignSharings();
    // TODO better choice
    delayedresharing::SharingMode plainSharing = delayedresharing::SharingMode::PLAIN;
    sharings.insert(plainSharing);
}

std::string delayedresharing::Constant::toSymbol()
{
    return std::to_string(value);
}