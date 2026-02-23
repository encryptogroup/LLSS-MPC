cmake_minimum_required(VERSION 3.15...3.30)
project(
  GeneralizedTerms
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)


if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_definitions(-DDEBUG) 
endif()


include_directories(.)


add_library(generalized_terms STATIC
    GeneralizedTerms/Util.cpp
    GeneralizedTerms/Ring.h
    GeneralizedTerms/Operation.cpp
    GeneralizedTerms/Symbol.cpp
    GeneralizedTerms/Symbol.h
    GeneralizedTerms/Serialization.cpp
    GeneralizedTerms/GeneralizedTerm.cpp
    GeneralizedTerms/GeneralizedTerm.h
    GeneralizedTerms/GeneralizedTermNetwork.cpp
    GeneralizedTerms/GeneralizedTermNetwork.h
    GeneralizedTerms/HyperGraph.h
    GeneralizedTerms/Definitions.cpp
    GeneralizedTerms/Definitions.h
  )
add_library(cost_metrics STATIC
    Metrics/CommunicationMetric/OfflineCommunicationMetric.cpp
    Metrics/CommunicationMetric/OnlineCommunicationMetric.cpp
    Metrics/CommunicationMetric/CommunicationMetric.h
    Metrics/DepthMetric/OfflineDepthMetric.cpp
    Metrics/DepthMetric/OnlineDepthMetric.cpp
    Metrics/DepthMetric/DepthMetric.h
    Metrics/ComputationMetric/OfflineComputationMetric.cpp
    Metrics/ComputationMetric/OnlineComputationMetric.cpp
    Metrics/ComputationMetric/ComputationMetric.h
    Metrics/SizeMetric/SizeMetric.h
    Metrics/CompositeMetric/CompositeMetric.cpp
    Metrics/CompositeMetric/CompositeMetric.h
    Metrics/Metric.h
    Metrics/CostModel.cpp
    Metrics/CostModel.h
    Metrics/AnalyticalThreePCCostModel.cpp
  )

add_library(optimizations STATIC
    Optimizations/OptimizationPass.h
    Optimizations/Partition.cpp
    Optimizations/Partition.h
    Optimizations/Ensemble.cpp
    Optimizations/Ensemble.h
    Optimizations/SequentialComposition.h
    Optimizations/ShareAssignment/ShareAssignment.h
    Optimizations/ShareAssignment/AstraShareAssignment.cpp
    Optimizations/ShareAssignment/GreedyShareAssignment.cpp
    Optimizations/ShareAssignment/BoundaryShareAssignment.cpp
    Optimizations/ShareAssignment/OfflineBoundaryShareAssignment.cpp
    Optimizations/ShareAssignment/PlainShareAssignment.cpp
  )

add_library(delayedresharing STATIC
    Delayedresharing.cpp
    Delayedresharing.h
)


target_link_libraries(delayedresharing cost_metrics optimizations generalized_terms)