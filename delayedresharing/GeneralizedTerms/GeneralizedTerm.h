#pragma once
#include "GeneralizedTerms/Symbol.h"
#include "GeneralizedTerms/Ring.h"
#include <functional>

namespace delayedresharing {

    // TODO: Terms should be rewritten into some operation representation LHS := RHS
    class GeneralizedTerm {
	    public:
            // TODO this should be per operation
            bool ring_is_commutative = true;
            // if lookup is the nullptr, copy the same symbols for input and output variables and create operations fresh, else create everything fresh
            // this allows us in ensemble to call clone and then drop-in replace the optimized clone for the original in a generalized Term Network if we have an empty lookup
            
            delayedresharing::GeneralizedTerm* clone(std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>* freshSymbolLookup);

            std::tuple<delayedresharing::GeneralizedTerm* , std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>> clonePartial();
            // Each node is a symbol in the directed hyper graph of computation
            // Assume Topological Ordering
            // Variables can only occur in interface: inputs and outputs
            std::vector<Symbol*> nodes;
           

            // INTERFACE
            // inputVariables U outputVariables ⊆ nodes

            // Inputs and Outputs Can only be Variables
            // ↓ Note these variables form the interface of a generalized Term and need to be carefully replaced upon any rewriting:
            // during rewriting of a generalized term all input variables and output variables need to be maintained, either copy them over or select used subset
            std::set<Variable*> inputVariables;
            std::set<Variable*> outputVariables;

            // mark modified if graph structure changes
            bool dirty = true;

            // lookup table to speed up
            std::map<delayedresharing::Symbol*,int> depth;

            // we only want to destroy operations and constants, as variables may be shared across GTs
            ~GeneralizedTerm()
            {
                for(auto node : nodes)
                {
                    if(typeid(*node) == typeid(Operation) || typeid(*node) == typeid(Constant))
                    {
                        delete node;
                    }
                }
            }            

    void exhaustiveDeduplicate(std::vector<std::vector<delayedresharing::Symbol*>>* bins);      
    void fuzzingDeduplicate(std::vector<std::vector<delayedresharing::Symbol*>>* bins);
    void syntacticDeduplicate();
    void semanticDeduplicate();
    void deduplicate();     
    void reportShareAssignment();

    void orderNodes();

    void toDot(std::string path);
            
            // Render down to a set of representative terms, from output down to inputs
            std::vector<std::string> Terms()
            {
                std::vector<std::string> terms;
                if(outputVariables.size() > 0)
                {
                    for( delayedresharing::Variable* outV : outputVariables)
                    {
                    std::string term = outV->render()+":=";
                    if(outV->input != nullptr)
                    {
                    term+= outV->input->render();

                    }else{
                    term += "?";
                    }
                    terms.push_back(term);
                    }
                }else{
                    // Must be the output delayedresharing:
                    for(auto inV: inputVariables)
                    {
                        terms.push_back("(OUTPUT):="+inV->render());
                    }
                }
                return terms;
            }

            std::string toString();

            static GeneralizedTerm* fromTerms(std::vector<std::string> terms)
            {
                return nullptr;
            }

            static GeneralizedTerm* fromFile(std::string path);

            void assignDefUses();

            // this requires that sets of all input and outputVariables are set
            void updateNodeList();
    };
}