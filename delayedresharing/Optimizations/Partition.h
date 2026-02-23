#pragma once
#include "OptimizationPass.h"
#include <memory>
namespace delayedresharing {

class Partition : public OptimizationPass {
public:
    int k = 1;
  virtual delayedresharing::GeneralizedTermNetwork * applyFresh( delayedresharing::GeneralizedTermNetwork *termNetwork);
};
}