#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <sstream>
#include <map>
#include <cassert>
#include <functional>
#include <algorithm>
#include <cmath>
#include <memory>

#include "GeneralizedTerms/HyperGraph.h"
#include "GeneralizedTerms/Definitions.h"
#include "GeneralizedTerms/Symbol.h"
#include "GeneralizedTerms/GeneralizedTerm.h"

namespace delayedresharing
{



    // This class is needed for some optimizations: it is the generalized term network without variables, flattened to operations
    class ComputationHyperGraph{
        public:
            int num_nodes = 0;
            std::map<int, delayedresharing::Operation*> index;
            std::vector<HyperEdge<int,int>*> edges;
            std::vector<uint> partition;
            void toDot(std::string path);
            ~ComputationHyperGraph()
            {
                for(const auto& edge : edges)
                {
                    delete edge;
                }
            }
    };

    class GeneralizedTermNetwork {
        public:
            // Note: Edges of the delayedresharingM are declared in each node inside of the delayedresharings but may cross from one delayedresharing into another
            GeneralizedTerm* inputs;
            GeneralizedTerm* outputs;

            // Generalized Terms form a natural partition of our delayedresharingM program, such that only a node that is an input and output variable somewhere, is contained in two seperate generalized terms
            std::vector<GeneralizedTerm*> generalizedTerms;

            ~GeneralizedTermNetwork()
            {
                std::set<delayedresharing::Symbol*> toDelete;
                for(const auto& t : generalizedTerms)
                {
                    for(const auto& n : t->nodes)
                    {
                        toDelete.insert(n);
                    }
                }

                for(const auto& n : toDelete)
                {
                    delete n;
                }
            }
            static GeneralizedTermNetwork* fromBristol(std::string path);
            static GeneralizedTermNetwork* fromFile(std::string path);
            
            void toDot(std::string path);

            void reportShareAssignment();

            std::vector<delayedresharing::Operation*> OperationsInTopologicalOrder();

            // Extract Computation Graph: This is needed for graph partitioning and term graph rewriting
            // remove all variables, fold only into computing operations, excluding
            ComputationHyperGraph* extractComputationGraph();

            // Assign forwards connections defined in defUses set of each symbol
            void assignDefUses();

            // Assign sharings in which the result of a node exists in sharings set of each symbol
            void assignSharings();

            void usageCheck()
            {
                for(const auto generalizedTerm : generalizedTerms)
                for(const auto node : generalizedTerm->nodes)
                {
                  if(node->defUses.size() == 0)
                  {
                  std::cout << "Node " << node->toSymbol() << " has no uses\n";
                  if(typeid(*node) != typeid(delayedresharing::Variable))
                  {
                    assert(false);
                  }
                  }
                }
            }

            void addMissingVariables()
            {
                for(auto generalizedTerm : generalizedTerms)
                {
                    generalizedTerm->nodes.insert(generalizedTerm->nodes.begin(),generalizedTerm->inputVariables.begin(),generalizedTerm->inputVariables.end());
                    generalizedTerm->nodes.insert(generalizedTerm->nodes.end(),generalizedTerm->outputVariables.begin(),generalizedTerm->outputVariables.end());
                }
            }

            // replace the jth generalized term with t (t must allready contain all the variables which are in the old GT)

            // j: place where to replace
            // t: new generalized term
            // SymbolLookUp: maps input operation of outputVariables to fresh Operations that produce the same result
            void replace(int j,GeneralizedTerm* t, std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>* SymbolLookup)
            {
                // link back output variables
                for(auto outVar : generalizedTerms[j]->outputVariables)
                {
                    outVar->input = (*SymbolLookup)[outVar];
                }
                
                generalizedTerms[j] = t;
                assignDefUses();
            }

            delayedresharing::GeneralizedTermNetwork* clone();

            void toFile(std::string path);

            std::string toString()
            {
                std::string result = "GenTermNetwork: \n";
                for(auto genTerm : generalizedTerms)
                {
                    result += genTerm->toString() +"\n\n";
                } 
                return result;
            }
    };

    void validateNodeSet(std::vector<delayedresharing::Symbol*> nodes);


    std::string NodeText( delayedresharing::Symbol* node);

}
