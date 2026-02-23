#include <GeneralizedTerms/Definitions.h>
#include <cstdint>

std::vector<std::string> delayedresharing::split(const std::string& str, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string delayedresharing::SharingModeName( delayedresharing::SharingMode mode)
{
    switch(mode)
    {
        case delayedresharing::SharingMode::BLINDED:
            return "Blinded";
        case delayedresharing::SharingMode::ADDITIVE:
            return "Additive";
        case delayedresharing::SharingMode::PLAIN:
            return "Plain";
        default:
            return "Unknown";
    }
}

std::string delayedresharing::pointer_to_string(void* ptr) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(sizeof(void*) * 2) << (uintptr_t)ptr; 
    return ss.str();
}

delayedresharing::OperationType delayedresharing::OperationTypeFromString(std::string string)
{
  if(string == "INPUT")
  {
    return delayedresharing::OperationType::INPUT;
  }
  if(string == "+")
  {
    return delayedresharing::OperationType::ADD;
  }
  if(string == "*")
  {
    return delayedresharing::OperationType::MULT;
  }
  if(string == "OUTPUT")
  {
    return delayedresharing::OperationType::OUTPUT;
  }
  return delayedresharing::OperationType::OUTPUT;
}



std::string delayedresharing::TensorTypeName(delayedresharing::TensorType tensorType)
{
    switch(tensorType)
    {
        case delayedresharing::TensorType::Scalar:
            return "Scalar";
        case delayedresharing::TensorType::Vector:
            return "Vector";
        case delayedresharing::TensorType::Matrix:
            return "Matrix";
        default:
            return "Unknown";
    }
}

std::string delayedresharing::ScalarTypeName(delayedresharing::ScalarType scalarType)
{
    switch(scalarType)
    {
        case delayedresharing::ScalarType::Integer:
            return "Integer";
        case delayedresharing::ScalarType::Decimal:
            return "Decimal";
        default:
            return "Unknown";
    }
}



delayedresharing::TensorType TensorTypeFromString(std::string tensorType)
{
    if(tensorType == "Scalar"){
        return delayedresharing::TensorType::Scalar;
    }else
    if(tensorType == "Vector"){
        return delayedresharing::TensorType::Vector;
    }else
    if(tensorType == "Matrix"){
        return delayedresharing::TensorType::Matrix;
    }
}

delayedresharing::ScalarType ScalarTypeFromString(std::string scalarType)
{
    
    if(scalarType == "Integer"){
        return delayedresharing::ScalarType::Integer;
    }else
    if(scalarType == "Decimal"){
        return delayedresharing::ScalarType::Decimal;
    }
}


delayedresharing::SharingMode delayedresharing::ModeFromString(std::string mode)
{
    if(mode == "Blinded")
    {
        return delayedresharing::SharingMode::BLINDED;
    }else
    if(mode == "Additive"){
        return delayedresharing::SharingMode::ADDITIVE;
    }else
    if(mode == "Plain"){
        return delayedresharing::SharingMode::PLAIN;
    }else
    {
        return delayedresharing::SharingMode::NONE;
    }
}
std::string delayedresharing::ValueTypeName( delayedresharing::ValueType valueType)
{
    return std::to_string(valueType.ringwidth)+","+TensorTypeName(valueType.tensorType)+","+ScalarTypeName(valueType.scalarType);
}

delayedresharing::ValueType delayedresharing::ValueTypeFromString(std::string string)
{
    delayedresharing::ValueType valueType;
    std::vector<std::string> parts = delayedresharing::split(string,',');
    valueType.ringwidth = std::stoi(parts[0]);
    valueType.tensorType = TensorTypeFromString(parts[1]);
    valueType.scalarType = ScalarTypeFromString(parts[2]);

    return valueType;
}

std::tuple<delayedresharing::SharingMode,delayedresharing::SharingMode> delayedresharing::SharingTupleFromString(std::string tuple)
{
    size_t split = tuple.find(":");
    std::string m_a = tuple.substr(0,split);
    std::string m_b = tuple.substr(split+1);
    delayedresharing::SharingMode a = ModeFromString(m_a);
    delayedresharing::SharingMode b = ModeFromString(m_b);
    return std::make_tuple(a,b);
}


