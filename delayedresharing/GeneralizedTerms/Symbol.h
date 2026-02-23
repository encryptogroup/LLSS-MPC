#pragma once
#include "GeneralizedTerms/Ring.h"

namespace delayedresharing {




        class Symbol : public delayedresharing::Node {
        private:
        
            
        public:
            // This is the set of all sharings in which this Symbol will exist at this point of protocol, including resharings of itself
            //
            // Note that an operation may be computed in multiple sharing types
            // Note that the computation may take place in one of those sharing types only if all children exist in that sharing type
            
            std::set<delayedresharing::SharingMode> sharings;

            // NOTE sharings and defUses will get correctly annotated upon sharing assignment
            
            //  ForwardConnections: Each def-use-hyperedge defines all symbols where this result is used
            std::set< delayedresharing::Symbol*> defUses;

            delayedresharing::ValueType valueType = {ringwidth: 1};
            
            // this should only contain all conversions needed to get all sharings in sharings from the sharings available from the defining place of this symbol
            std::set<std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode>> conversions;

            void addConversion(std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
            {
                conversions.insert(conversion);
                sharings.insert(std::get<1>(conversion));
            }


            // Sharings must match possible configurations
            virtual std::string render();

            std::vector<delayedresharing::Symbol*> InputSymbols();

            virtual int SymbolDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
            {
                // you should define Symbol depth for the symbol you have or not use the symbol class directly
                assert(false);
                return 1;
            }

            virtual int MultiplicativeDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
            {
                // you should define Symbol depth for the symbol you have or not use the symbol class directly
                assert(false);
                return 1;
            }


            virtual std::string toSymbol();

            virtual bool hasSharing(delayedresharing::SharingMode sharing)
            {
                return (sharings.count(sharing) > 0);
            }


            bool hasConversionToMode(delayedresharing::SharingMode test)
            {
                if(conversions.empty())
                {
                    return false;
                }
                for(auto conversion : conversions)
                {
                    if(std::get<1>(conversion)== test)
                    {
                        return true;
                    }
                }
                return false;
            }

            virtual std::string reportShareAssignment();

            virtual void assignSharings();

            
            delayedresharing::Symbol* clone(std::map<delayedresharing::Symbol*,delayedresharing::Symbol*>* freshSymbolLookup);

            virtual int evaluate(std::map<delayedresharing::Symbol*,int>* lookup)
            {
                RingAdd<int>(1,1,1);
                assert(false);
                return 0;
            }

        };

        class Operation : public Symbol {
            public:
            // Defines input Symbols used in computation in correct order
            std::vector<delayedresharing::Symbol*> inputs;

            delayedresharing::OperationType operationType;

            std::tuple<delayedresharing::ValueType,delayedresharing::ValueType> signature = { {ringwidth: 1}, {ringwidth: 1}};

            virtual int SymbolDepth(std::map<delayedresharing::Symbol*,int>* depthMap = nullptr);
            
            virtual int MultiplicativeDepth(std::map<delayedresharing::Symbol*,int>* depthMap = nullptr);

            // Specifies all operation input share types with which this operation is supposed to be performed for example fur operationtype=MULT this would contain (SharingMode::BLINDED,SharingMode::ADDITIVE) in the 3 input case
            // Note sharings[i] = operationSharings[i][2]
            std::set<std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode>> operationSharings;
            

            virtual bool hasOperationSharing(std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> opSharing)
            {
                return (operationSharings.count(opSharing));
            }

            void addOperationSharing(std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> opSharing)
            {
                operationSharings.insert(opSharing);
                sharings.insert(std::get<1>(opSharing));
            }

            virtual std::string toSymbol()
            {
                switch(operationType)
                    {
                            break;
                        case delayedresharing::OperationType::ADD:
                            return "+";
                            break;
                        case delayedresharing::OperationType::MULT:
                            return "*";
                            break;
                        case delayedresharing::OperationType::DOTPRODUCT:
                            return "d*";
                            break;
                        case delayedresharing::OperationType::INPUT:
                            return "INPUT";
                            break;
                        case delayedresharing::OperationType::OUTPUT:
                            return "OUTPUT";
                            break;
                        default:
                            return "?";
                            break;
                    }
            }

            virtual std::string render();
            
            virtual void assignSharings();
            
            bool isCommutative();

            std::vector<delayedresharing::Symbol*> orderedInputs();

            std::vector<delayedresharing::Operation*> findInputOperations();
            
            std::vector<delayedresharing::Operation*> findNextOperations();


        };

        // TODO Where does variable get its definition? Either none (input variable) or exactly one Symbol (intermediate/output variable)
        class Variable : public Symbol {
            public:
                Symbol* input = nullptr;
                std::string variableName = "undefined";

                virtual std::string toSymbol()
                {
                    return "v"+variableName;
                }

                virtual int SymbolDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
                {
                    if(input != nullptr)
                    {
                        return input->SymbolDepth(depthMap);
                    }else
                    {
                        return 0;
                    }
                }
                
                virtual int MultiplicativeDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
                {
                    if(input != nullptr)
                    {
                        return input->MultiplicativeDepth(depthMap);
                    }else
                    {
                        return 0;
                    }
                }

                virtual void assignSharings();
        };

        class Constant : public Symbol {
            public:
                int value = 0;
                virtual std::string toSymbol();
                
                // constants dont add any real symbol depth
                virtual int SymbolDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
                {
                    return 0;
                }

                virtual int MultiplicativeDepth(std::map<delayedresharing::Symbol*,int>* depthMap)
                {
                    return 0;
                }

                virtual void assignSharings();
        };

}