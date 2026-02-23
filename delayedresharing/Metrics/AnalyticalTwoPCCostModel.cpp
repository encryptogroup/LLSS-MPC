// ASTRA like 2PC + helper costs with delayed resharing

// a cost point should only override its layer depth when needed

#include "Metrics/CostModel.h"

int cost_of_gamma_gen_computation(int k)
{
    return 2* delayedresharing::BaseCosts::PRF + (k+1)* delayedresharing::BaseCosts::Addition + k*( delayedresharing::BaseCosts::Multiplication);
}
int cost_of_lambda_gen_computation()
{
    return 4* delayedresharing::BaseCosts::PRF;
}


delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OfflineCommunicationConversion(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
{
    delayedresharing::CostPoint costPoint;
            auto in = std::get<0>(conversion);
            auto out = std::get<1>(conversion);
            switch(in)
            {
                case delayedresharing::SharingMode::ADDITIVE:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::BLINDED:
                                // lambda gen is non interactive
                                return costPoint;
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                        }
                    break;
                case delayedresharing::SharingMode::BLINDED:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                            case delayedresharing::SharingMode::BLINDED:
                                return costPoint;
                        }
                    break;
            }
            // Cost could not be assigned
            assert(false);
            return costPoint;
}
delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OnlineCommunicationConversion(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
{
        delayedresharing::CostPoint costPoint;
            auto in = std::get<0>(conversion);
            auto out = std::get<1>(conversion);
            switch(in)
            {
                case delayedresharing::SharingMode::ADDITIVE:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::BLINDED:
                                // one lambda gen communication from input width
                                costPoint.cost = 2*symbol->valueType.ringwidth; 
                                return costPoint;
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                        }
                    break;
                case delayedresharing::SharingMode::BLINDED:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                            case delayedresharing::SharingMode::BLINDED:
                                return costPoint;
                        }
                    break;
            }
            // Cost could not be assigned
            assert(false);
            return costPoint;
}

delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OfflineCommunicationOperation( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
{
    delayedresharing::CostPoint costPoint;
        if(typeid(*symbol) == typeid(Operation)){
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) symbol);
            delayedresharing::OperationType operationType = operation->operationType;
            delayedresharing::SharingMode inputSharingMode = std::get<0>(mode);
            delayedresharing::SharingMode outputSharingMode = std::get<1>(mode);
            int k = operation->inputs.size();
            int k_constant = 0;

            for(auto input : operation->inputs)
            {
                if(typeid(*input) == typeid( delayedresharing::Constant))
                {
                    k_constant++;
                }
            }

            int k_variable = k - k_constant;

            switch(operationType)
            {
                case delayedresharing::OperationType::ADD:
                {
                    return costPoint;
                }
                case delayedresharing::OperationType::MULT:
                {
                    // Needs multiple inputs
                    assert(k > 1);
                    if(k_variable < 2)
                    {
                        return costPoint;
                    }
                    
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);

                    int pairs = 1;
                    for(int i = 0;i<k_variable;i++)
                    {
                        pairs *= 2;
                    }
                    int ops = (pairs-k_variable-1);
                    int gamma_gen_comm = symbol->valueType.ringwidth;
                    // TODO: costs for local mults?
                    costPoint.cost = ops*gamma_gen_comm;
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::DOTPRODUCT:
                {
                    // Needs multiple inputs
                    assert(k > 1);
                    
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);
                    // only one ring element needed in setup
                    costPoint.cost = symbol->valueType.ringwidth;
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::INPUT:
                {
                    //lambda gen has no comm in 3pc
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::OUTPUT:
                {
                    return costPoint;
                    break;
                }
            }
        }
    assert(false);
    return costPoint;
}
delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OnlineCommunicationOperation( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
{
    delayedresharing::CostPoint costPoint;
        if(typeid(*symbol) == typeid(Operation)){
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) symbol);
            delayedresharing::OperationType operationType = operation->operationType;
            delayedresharing::SharingMode inputSharingMode = std::get<0>(mode);
            delayedresharing::SharingMode outputSharingMode = std::get<1>(mode);
            int k = operation->inputs.size();
            int k_constant = 0;

            for(auto input : operation->inputs)
            {
                if(typeid(*input) == typeid( delayedresharing::Constant))
                {
                    k_constant++;
                }
            }

            int k_variable = operation->findInputOperations().size();
            switch(operationType)
            {
                case delayedresharing::OperationType::ADD:
                {
                        return costPoint;
                        break;
                }
                case delayedresharing::OperationType::MULT:
                {
                    // Needs multiple inputs
                    assert(k > 1);
                    if(k_variable<=1)
                    {
                        // we claim here that it is free as it in essence is
                        return costPoint;
                    }
                    
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);
                    // no comm
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::DOTPRODUCT:
                {
                    // Needs multiple inputs
                    assert(k > 1);
                    if(k_variable<=1)
                    {
                        // we claim here that it is free as it in essence is
                        return costPoint;
                    }
                    
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);
                    // no comm
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::INPUT:
                {
                    costPoint.cost = symbol->valueType.ringwidth;
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::OUTPUT:
                {
                    costPoint.cost = symbol->valueType.ringwidth;
                    return costPoint;
                    break;
                }
            }
        }
    assert(false);
    return costPoint;
}


delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OfflineComputationOperation( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
{
    delayedresharing::CostPoint costPoint;
        if(typeid(*symbol) == typeid(Operation)){
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) symbol);
            delayedresharing::OperationType operationType = operation->operationType;
            delayedresharing::SharingMode inputSharingMode = std::get<0>(mode);
            delayedresharing::SharingMode outputSharingMode = std::get<1>(mode);
            int k = operation->inputs.size();
            int k_constant = 0;

            for(auto input : operation->inputs)
            {
                if(typeid(*input) == typeid( delayedresharing::Constant))
                {
                    k_constant++;
                }
            }

            int k_variable = k - k_constant;
            switch(operationType)
            {
                case delayedresharing::OperationType::ADD:
                {
                        int costOfAddition = (k-1)*((inputSharingMode == delayedresharing::SharingMode::ADDITIVE)?0:(4*((int)BaseCosts::Addition)));
                        
                        costPoint.cost = costOfAddition;
                        return costPoint;
                        break;
                }
                case delayedresharing::OperationType::MULT:
                {
                    // Needs multiple inputs
                    assert(k > 1);
                    if(k_variable<=1)
                    {
                        // we claim here that it is free as it in essence is
                        return costPoint;
                    }
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);

                    int pairs = 1;
                    for(int i = 0;i<k_variable;i++)
                    {
                        pairs *= 2;
                    }
                    int ops = (pairs-k_variable-1);

                    costPoint.cost = ops*(cost_of_gamma_gen_computation(k_variable)+ delayedresharing::BaseCosts::Addition+ delayedresharing::BaseCosts::Multiplication);
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::INPUT:
                {
                    costPoint.cost = cost_of_lambda_gen_computation();
                    return costPoint;
                    break;
                }
                
                case delayedresharing::OperationType::OUTPUT:
                {
                    return costPoint;
                    break;
                }
            }
        }
    assert(false);
    return costPoint;
}
delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OnlineComputationOperation( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
{
    delayedresharing::CostPoint costPoint;
        if(typeid(*symbol) == typeid(Operation)){
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) symbol);
            delayedresharing::OperationType operationType = operation->operationType;
            delayedresharing::SharingMode inputSharingMode = std::get<0>(mode);
            delayedresharing::SharingMode outputSharingMode = std::get<1>(mode);
            int k = operation->inputs.size();
            int k_constant = 0;

            for(auto input : operation->inputs)
            {
                if(typeid(*input) == typeid( delayedresharing::Constant))
                {
                    k_constant++;
                }
            }

            int k_variable = k - k_constant;

            switch(operationType)
            {
                case delayedresharing::OperationType::ADD:
                {
                        costPoint.cost = (k-1)*2*((int)BaseCosts::Addition);
                        return costPoint;
                        break;
                }
                case delayedresharing::OperationType::MULT:
                {

                    // Needs multiple inputs
                    assert(k > 1);
                    
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);
                    //TODO cost: for every input, every party has 2 values, multiply every pair of them and sum them
                    // only one addition is omitted because at the end we have two arithmetic shares
                    int pairs = 1;
                    for(int i = 0;i<k_variable;i++)
                    {
                        pairs *= 2;
                    }
                    int number_of_multiplications = pairs - 1;
                    int number_of_additions = pairs - 1;
                    int cost = number_of_multiplications* ((int) delayedresharing::BaseCosts::Multiplication) + number_of_additions* ((int) delayedresharing::BaseCosts::Addition);
                    costPoint.cost = 2*cost;
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::INPUT:
                {
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    costPoint.cost = ((int) delayedresharing::BaseCosts::Addition);
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::OUTPUT:
                {
                    if(inputSharingMode == delayedresharing::SharingMode::BLINDED)
                    {
                        costPoint.cost = 2*((int) delayedresharing::BaseCosts::Addition);
                        return costPoint;
                    }else 
                    if(inputSharingMode == delayedresharing::SharingMode::ADDITIVE)
                    {
                        costPoint.cost = ((int) delayedresharing::BaseCosts::Addition);
                        return costPoint;
                    }
                    assert(false);
                    break;
                }
                default:
                {
                    // Cost could not be assigned
                    assert(false);
                    return costPoint;
                }
            }
        }
    assert(false);
    return costPoint;
}

delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OfflineComputationConversion(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
{
            delayedresharing::CostPoint costPoint;
            auto in = std::get<0>(conversion);
            auto out = std::get<1>(conversion);
            switch(in)
            {
                case delayedresharing::SharingMode::ADDITIVE:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::BLINDED:
                                costPoint.cost = cost_of_lambda_gen_computation();
                                return costPoint;
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                        }
                    break;
                case delayedresharing::SharingMode::BLINDED:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                            case delayedresharing::SharingMode::BLINDED:
                                return costPoint;
                        }
                    break;
            }
    // Cost could not be assigned
    assert(false);
    return costPoint;
}

delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OnlineComputationConversion(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
{
            delayedresharing::CostPoint costPoint;
            auto in = std::get<0>(conversion);
            auto out = std::get<1>(conversion);
            switch(in)
            {
                case delayedresharing::SharingMode::ADDITIVE:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::BLINDED:
                                costPoint.cost = 2*((int) BaseCosts::Addition);
                                return costPoint;
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                        }
                    break;
                case delayedresharing::SharingMode::BLINDED:
                        switch(out)
                        {
                            case delayedresharing::SharingMode::ADDITIVE:
                                costPoint.cost = 2*( ( (int) BaseCosts::Check) + ( (int) BaseCosts::Addition)+( (int) BaseCosts::Multiplication));
                                return costPoint;
                            case delayedresharing::SharingMode::BLINDED:
                                return costPoint;
                        }
                    break;
            }
    // Cost could not be assigned
    assert(false);
    return costPoint;
}


delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OfflineDepthConversion(delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
{
    delayedresharing::CostPoint costPoint;
    auto in = std::get<0>(conversion);
    auto out = std::get<1>(conversion);
    costPoint.layerAssigned = true;
    switch(in)
    {
        case delayedresharing::SharingMode::ADDITIVE:
        {
                        switch(out)
                        {
                            case delayedresharing::SharingMode::BLINDED:
                                // all resharing setup rounds happen at the same time in the initial layer // there are no dependencies
                                // Note that lambda gen has no interaction in 3PC
                                costPoint.cost = 0;
                                costPoint.layer = 0;
                                return costPoint;
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                        }
        }
        break;
        case delayedresharing::SharingMode::BLINDED:
        {
                        switch(out)
                        {
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                            case delayedresharing::SharingMode::BLINDED:
                                return costPoint;
                        }
        }
        break;
    }
    assert(false);
    return costPoint;
}
delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OnlineDepthConversion(delayedresharing::Symbol* symbol, std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> conversion)
{
    delayedresharing::CostPoint costPoint;
    costPoint.layerAssigned = true;
    auto in = std::get<0>(conversion);
    auto out = std::get<1>(conversion);
    switch(in)
    {
        case delayedresharing::SharingMode::ADDITIVE:
        {
                        switch(out)
                        {
                            case delayedresharing::SharingMode::BLINDED:

                                // resharings from arithmetic to blinded are interactive rounds
                                costPoint.cost = 1;
                                costPoint.layerAssigned = false;
                                return costPoint;
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                        }
        }
        break;
        case delayedresharing::SharingMode::BLINDED:
        {
                        switch(out)
                        {
                            case delayedresharing::SharingMode::ADDITIVE:
                                return costPoint;
                            case delayedresharing::SharingMode::BLINDED:
                                return costPoint;
                        }
        }
        break;
    }
    assert(false);
    return costPoint;
}


delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OfflineDepthOperation( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
{
    delayedresharing::CostPoint costPoint;
    costPoint.layerAssigned = true;
        if(typeid(*symbol) == typeid(Operation)){
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) symbol);
            delayedresharing::OperationType operationType = operation->operationType;
            delayedresharing::SharingMode inputSharingMode = std::get<0>(mode);
            delayedresharing::SharingMode outputSharingMode = std::get<1>(mode);
            int k = operation->inputs.size();
            int k_constant = 0;

            for(auto input : operation->inputs)
            {
                if(typeid(*input) == typeid( delayedresharing::Constant))
                {
                    k_constant++;
                }
            }

            int k_variable = k - k_constant;
            switch(operationType)
            {
                case delayedresharing::OperationType::ADD:
                {
                        return costPoint;
                        break;
                }
                case delayedresharing::OperationType::MULT:
                {

                    // Needs multiple inputs
                    assert(k > 1);
                    
                    
                    if(k_variable <= 1)
                    {
                        return costPoint;
                    }

                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);

                    // every multiplications cross pairs require the input lambda values so this must happen after the previous layer which is lambda computation
                    // needs interaction from helper to party
                    costPoint.cost = 1;
                    costPoint.layer = 1;
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::INPUT:
                {
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    // all resharing setup rounds happen at the same time in the initial layer // there are no dependencies
                    // in three PC lambda gen for setup happens non interactively            
                    costPoint.cost = 0;
                    costPoint.layer = 0;
                    // input lambda creation also happens on first round
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::OUTPUT:
                {
                    return costPoint;
                    break;
                }
                default:
                {
                    // Cost could not be assigned
                    assert(false);
                    return costPoint;
                }
            }
        }
    assert(false);
    return costPoint;
}

delayedresharing::CostPoint delayedresharing::AnalyticalThreePCCostModel::OnlineDepthOperation( delayedresharing::Symbol* symbol,std::tuple< delayedresharing::SharingMode, delayedresharing::SharingMode> mode)
{
    delayedresharing::CostPoint costPoint;
    costPoint.layerAssigned = true;
        if(typeid(*symbol) == typeid(Operation)){
            delayedresharing::Operation* operation = (( delayedresharing::Operation*) symbol);
            delayedresharing::OperationType operationType = operation->operationType;
            delayedresharing::SharingMode inputSharingMode = std::get<0>(mode);
            delayedresharing::SharingMode outputSharingMode = std::get<1>(mode);
            int k = operation->inputs.size();
            int k_constant = 0;

            for(auto input : operation->inputs)
            {
                if(typeid(*input) == typeid( delayedresharing::Constant))
                {
                    k_constant++;
                }
            }

            int k_variable = k - k_constant;
            switch(operationType)
            {
                case delayedresharing::OperationType::ADD:
                {
                        return costPoint;
                        break;
                }
                case delayedresharing::OperationType::MULT:
                {

                    // Needs multiple inputs
                    assert(k > 1);
                    
                    if(k_variable <= 1)
                    {
                        return costPoint;
                    }
                    // We only multiply in blinded shares and output arithmetic shares
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    assert(outputSharingMode == delayedresharing::SharingMode::ADDITIVE);
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::INPUT:
                {
                    assert(inputSharingMode == delayedresharing::SharingMode::BLINDED);
                    // online input sharing of m value is allways first round so first layer
                    costPoint.cost = 1;
                    costPoint.layer = 0;
                    costPoint.layerAssigned = true;
                    return costPoint;
                    break;
                }
                case delayedresharing::OperationType::OUTPUT:
                {
                    // in both sharings this takes one round
                    // need to compute depth according to circuit representation: use helper function
                    costPoint.cost = 1;
                    costPoint.layerAssigned = false;
                    return costPoint;
                    break;
                }
                default:
                {
                    // Cost could not be assigned
                    assert(false);
                    return costPoint;
                }
            }
        }
    assert(false);
    return costPoint;

}
