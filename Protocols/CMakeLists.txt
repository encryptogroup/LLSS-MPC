cmake_minimum_required(VERSION 3.15...3.30)
project(
  GeneralizedTerms
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
add_compile_options(-Wno-everything -mavx2 -msse4.2 -maes -msse2 -msse3 -mssse3 -msse4.1 -mpclmul )

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    add_definitions(-DDEBUG) 
endif()

include_directories(../thirdparty/kahypar/include)

find_package(libOTe HINTS ../thirdparty/libOTe)

include_directories(.)
include_directories(../delayedresharing)
link_directories(../delayedresharing/build)


add_library(protocol STATIC
  Protocols/Protocol.cpp
  Protocols/Protocol.h
  Protocols/MaskedProtocol.h
  Protocols/MaskedParty.cpp
  Protocols/MaskedParty.h
  Protocols/AbyParty.cpp
  Protocols/AbyParty.h
  Protocols/AstraParty.cpp
  Protocols/AstraParty.h
  Protocols/AstraHelper.cpp
  Protocols/AstraHelper.h
  Protocols/ProtocolDefinitions.h
  Protocols/RSSParty.h
  Protocols/Domain.h
)



target_link_libraries(protocol delayedresharing oc::libOTe oc::cryptoTools generalized_terms optimizations)

add_executable(DelayedresharingProtocol main_protocol.cpp main_protocol.h)

target_link_libraries(DelayedresharingProtocol protocol)
