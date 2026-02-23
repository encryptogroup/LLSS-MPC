#pragma once
#include "GeneralizedTerms/GeneralizedTermNetwork.h"
namespace delayedresharing {

    class OrderedSubtermElimination : public OptimizationPass {
        public:
            virtual void apply( delayedresharing::GeneralizedTerm* generalizedTerm)
            {
                // 1: extract subgraphs for each output variable of generalizedTerm

                // 2: order them according to Metric M
                // 3: build up annotated equivalence tree
                // 3: compare each pair of subgraphs according to order like in full sort
                //  4: check for subgraph equivalence and take that of smaller one in order, merge in remaining nodes

                // Idea: Do this with smt solving instead
                return;
            }
    };
}