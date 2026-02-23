#include "Optimizations/Ensemble.h"

        void delayedresharing::Ensemble::apply( delayedresharing::GeneralizedTermNetwork* termNetwork)
        {
            bool evaluateGTN = false;
            // Apply ensemble improvement to each generalized term in place
            for(int j = 1;j<(termNetwork->generalizedTerms.size()-1 );j++)
            {
                std::cout << "Working on Ensemble GT: " << j << "/"  << std::to_string(termNetwork->generalizedTerms.size()-2) << "\n";
                delayedresharing::GeneralizedTerm* inputTerm = termNetwork->generalizedTerms[j];
                std::cout << inputTerm->nodes.size() << " Nodes\n";
                testingAssignment->apply(termNetwork);

                float bestValue = 0;
                if(evaluateGTN)
                {
                    bestValue = metric->compute(termNetwork);
                }else
                {
                    bestValue = metric->compute(inputTerm);
                }

                std::cout << "Initial Value " << bestValue << "\n"; 
                

                std::map<delayedresharing::Symbol*,delayedresharing::Symbol*> oldInputLookup;
                for(const auto& outVar : inputTerm->outputVariables)
                {
                    oldInputLookup[outVar] = outVar->input;
                }
                
                std::vector<std::tuple<delayedresharing::GeneralizedTerm*,std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>>> results;
                
                
                results.push_back(std::make_tuple(inputTerm,oldInputLookup));
                int best_method = 0;

                for(int i = 0;i<methods.size();i++)
                {
                    auto method = methods[i];
                    // create clone of input term
                    auto clone = inputTerm->clonePartial();

                    delayedresharing::GeneralizedTerm* workingCopy = std::get<0>(clone);
                    std::map<delayedresharing::Symbol*,delayedresharing::Symbol*> initialInputLookup = std::get<1>(clone);
                    termNetwork->replace(j,workingCopy,&initialInputLookup);
                    // run method on it
                    method->metric = metric;
                    method->apply(workingCopy);
                    
                    #ifdef DEBUG
                    workingCopy->toDot("dotFiles/"+std::to_string(j)+"_"+method->name+"_dup.dot");
                    #endif
                    workingCopy->deduplicate();
                    
                    #ifdef DEBUG
                    workingCopy->toDot("dotFiles/"+std::to_string(j)+"_"+method->name+"_undup.dot");
                    #endif

                    std::map<delayedresharing::Symbol*,delayedresharing::Symbol*> newInputLookup;
                    for(const auto& outputVar : workingCopy->outputVariables)
                    {
                        newInputLookup[outputVar] = outputVar->input;
                    }
                    
                    results.push_back(std::make_tuple(workingCopy,newInputLookup));

                    
                    
                    // check if best
                    testingAssignment->apply(termNetwork);
                    float newValue = 0;
                    if(evaluateGTN)
                    {
                        newValue = metric->compute(termNetwork);
                    }else
                    {
                        newValue = metric->compute(workingCopy);
                    }

                    
                    // if best: add old best to toFree
                    if(newValue < bestValue)
                    {
                        bestValue = newValue;
                        std::cout << "Improvement found with " << method->name <<": " << bestValue << "\n";
                        best_method = i+1;
                    }
                    termNetwork->replace(j,inputTerm,&oldInputLookup);
                }

                // finally apply best method

                auto best_result = results[best_method];
                termNetwork->replace(j,std::get<0>(best_result),&std::get<1>(best_result));

                // discard old ones
                for(int i = 0;i<results.size();i++)
                {
                    if(i != best_method)
                        delete std::get<0>(results[i]);
                }
            }

            std::vector<delayedresharing::GeneralizedTerm*> emptyGTs;
            
            for(int j = 1;j<(termNetwork->generalizedTerms.size()-1 );j++)
            {
                // todo: this should be disconnect instead
                if( ( termNetwork->generalizedTerms[j]->nodes.size() == 0 ) || ( termNetwork->generalizedTerms[j]->outputVariables.size() == 0 ) )
                {
                    emptyGTs.push_back(termNetwork->generalizedTerms[j]);
                }
            }

            for(const auto gt : emptyGTs)
            {
                auto it = std::remove(termNetwork->generalizedTerms.begin(), termNetwork->generalizedTerms.end(), gt);
                termNetwork->generalizedTerms.erase(it,termNetwork->generalizedTerms.end());
                
                #ifdef DEBUG
                std::cout << "Empty GT " << gt << " was removed after ensemble pass\n";
                #endif
                delete gt;
            }

            std::cout << "Finished ensemble pass" << "\n";
        }