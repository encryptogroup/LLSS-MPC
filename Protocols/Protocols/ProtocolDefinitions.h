#pragma once

#include <unordered_map>
#include <queue>
#include <cstddef>
#include <vector>
#include <cstdint>

#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Matrix.h>
#include <cryptoTools/Common/MatrixView.h>
#include <coproto/Socket/AsioSocket.h>
#include <coproto/coproto.h>
#include <string>


typedef osuCrypto::u64 RingElement; 



RingElement DrawRingElement(osuCrypto::PRNG& prng, RingElement ringSize)
{
    RingElement out = prng.get<RingElement>();
    while(out < 0)
    {
        out +=ringSize;
    }
    return (out % ringSize);
}

int hammingWeight(uint32_t x) {
  return __builtin_popcount(x); 
}