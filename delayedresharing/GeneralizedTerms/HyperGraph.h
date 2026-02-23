#pragma once
#include "GeneralizedTerms/Definitions.h"

namespace delayedresharing
{

    template <typename T,typename U> 
    class HyperEdge {
        public:
            std::vector<T> members;
            U annotation;
    };

    template <typename T,typename U> 
    class FlowEdge {
        public:
            T a;
            T b;
            U capacity;
            U flow;
    };




    class Node {
        public:
            delayedresharing::OperationType operationType;
    };
    class FlowNode : public Node {
        public:
                std::vector<delayedresharing::FlowEdge<delayedresharing::FlowNode*,int>*> next;
    };
}