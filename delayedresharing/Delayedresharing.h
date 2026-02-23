#pragma once

#include <GeneralizedTerms/GeneralizedTermNetwork.h>
#include <Optimizations/ShareAssignment/ShareAssignment.h>
#include <Optimizations/Ensemble.h>
#include <Optimizations/Partition.h>
#include <Optimizations/SequentialComposition.h>
#include <Metrics/ComputationMetric/ComputationMetric.h>
#include <Metrics/CommunicationMetric/CommunicationMetric.h>
#include <Metrics/DepthMetric/DepthMetric.h>
#include <Metrics/SizeMetric/SizeMetric.h>
#include <Metrics/CompositeMetric/CompositeMetric.h>
#include <Metrics/CostModel.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <algorithm>

namespace delayedresharing
{
    class Delayedresharing{
        public:
            std::string inputFile = "";
            std::string outputFile = "";

            int generalizedTermSize = 1;
            int iterations = 1;
            
            delayedresharing::CompositeMetric* compositeMetric = new delayedresharing::CompositeMetric();

            delayedresharing::CostModel* costModel;

            bool initial_deduplicate = true;

            std::vector<delayedresharing::ShareAssignment*> shareAssignments;
            int initialAssignment = 0; // assignment to determine costs without any optimizations
            int testingAssignment = 0; // assignment to check for improvement in ensemble and rewritingrules
            int finalAssignment = 0; // assignment to determine final costs
            
            delayedresharing::Ensemble* logicEnsemble = new delayedresharing::Ensemble();
            std::vector<delayedresharing::ShareAssignment*> shareEnsemble; // subset of shareAssignments
            float initial_value;
            float final_value;
            void run();
    };
}