#pragma once
#include <tuple>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>

namespace delayedresharing
{

    // note these are generic operations and dont have an explict meaning predefined (you can add operations as you wish)
    // giving these operations semantics and rewriting rules is a per protocol choice
    // here we give descriptions on the basis of the DELAYEDRESHARING protocol modifications
    enum OperationType {
        UNDEFINED,
        INPUT, // input sharing operation
        MULT, // takes k inputs and outputs their product
        ADD, // takes k inputs and outputs their sum 
        DOTPRODUCT, // takes k input vectors and returns their multilinear dot product
        BUNDLE, // takes d inputs and turns them into next higher rank tensor with d components
        SPLIT, // takes one k rank tensor T and one scalar i and returns T[i]
        OUTPUT // outputs plain value of a sharing
    };


    
    // TODO: this should be coupled somewhat with Ring.h
    // defines in what structure shares of a Ring are aligned in (these structures may again form a ring under some other form of computation)
    enum TensorType {
        Scalar, // this supports: INPUT, OUTPUT, ADD, MULT
        Vector, // this supports: DOTPRODUCT, ADD
        Matrix // this supports: ADD, MULT
    };

    // TODO: this is the distinction between regular integer shares and those which have decimal places, i.e. require truncation
    // currently this does nothing but should in the future be utilized in sharing assignments and the protocol
    enum ScalarType {
        Integer,
        Decimal
    };

    struct ValueType {
        int ringwidth;
        TensorType tensorType = TensorType::Scalar;
        ScalarType scalarType = ScalarType::Integer;
    };

    enum SharingMode {
        NONE, // currently unassigned
        PLAIN, // x publicly known
        ADDITIVE, // x = x_1 + x_2
        BLINDED, // x = m_x + l_x^1 + l_x^2
        BOOLEAN, // x = x_1 ⊕ x_2
        YAO // x = Dec(~x) TODO: this currently does nothing
    };

    delayedresharing::SharingMode ModeFromString(std::string mode);

    std::string SharingModeName(delayedresharing::SharingMode mode);

    std::string ValueTypeName(delayedresharing::ValueType type);

    std::string TensorTypeName(delayedresharing::TensorType tensorType);
    std::string ScalarTypeName(delayedresharing::ScalarType type);

    delayedresharing::OperationType OperationTypeFromString(std::string string);
    delayedresharing::ValueType ValueTypeFromString(std::string string);
    
    std::tuple<delayedresharing::SharingMode,delayedresharing::SharingMode> SharingTupleFromString(std::string);

    std::string pointer_to_string(void* ptr);

    std::vector<std::string> split(const std::string& str, char delim);
}