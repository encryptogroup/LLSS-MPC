#pragma once
#include "OptimizationPass.h"


namespace delayedresharing {

    class SequentialComposition : public OptimizationPass {
        public:

            std::vector<delayedresharing::OptimizationPass*> passes;
            
            virtual delayedresharing::GeneralizedTermNetwork* applyFresh( delayedresharing::GeneralizedTermNetwork* termNetwork = nullptr)
            {
                delayedresharing::GeneralizedTermNetwork* newNetwork = passes[0]->applyFresh(termNetwork);
                for(int i = 1; i < passes.size();i++)
                {
                    passes[i]->apply(newNetwork);
                }
                return newNetwork;
            }

            // Applies optimization to this GTN

            virtual void apply( delayedresharing::GeneralizedTermNetwork* termNetwork = nullptr)
            {
                for(const auto& pass : passes)
                {
                    pass->apply(termNetwork);
                }
            }

            // Applies optimization to only this GT

            virtual void apply( delayedresharing::GeneralizedTerm* generalizedTerm = nullptr)
            {
                for(const auto& pass : passes)
                {
                    pass->apply(generalizedTerm);
                }
            }
    };
}