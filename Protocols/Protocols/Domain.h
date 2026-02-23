#pragma once
#include <vector>
#include "Protocols/ProtocolDefinitions.h"

namespace delayedresharing {
    class DomainValue {
        public:
        
        void print(){
        }

        DomainValue operator+(const DomainValue& other) const {
            DomainValue r;
            return r;
        }

        
        DomainValue addConst(int c) const {
            DomainValue r;
            return r;
        }

        DomainValue multConst(int c) const {
            DomainValue r;
            return r;
        }
        
        DomainValue operator-(const DomainValue& other) const {
            DomainValue r;
            return r;
        }
        
        DomainValue operator*(const DomainValue& other) const {
            DomainValue r;
            return r;
        }
        DomainValue operator-() const {
            DomainValue r;
            return r;
        }
    };

    class BooleanValue : public DomainValue {
        public:

        BooleanValue()
        {

        }

        osuCrypto::BitVector v;


        BooleanValue(osuCrypto::u64 s)
        {
            v = osuCrypto::BitVector(s);
        }

        void print(){
            std::cout << v;
        }

        BooleanValue operator+(const BooleanValue& other) const {
            BooleanValue r;
            r.v = this->v ^ other.v;
            return r;
        }

        BooleanValue addConst(bool c) {
            BooleanValue r;
            if(c)
            {
                r.v = ~this->v;
            }else
            {
                r.v = this->v;
            }
            return r;
        }

        BooleanValue multConst(bool c) {
            BooleanValue r;
            if(c)
            {
                r.v = this->v;
            }else{
                r.v = osuCrypto::BitVector(this->v.size());
            }
            return r;
        }
        
        BooleanValue operator-(const BooleanValue& other) const {
            BooleanValue r;
            r.v = this->v ^ other.v;
            return r;
        }
        
        BooleanValue operator*(const BooleanValue& other) const {
            BooleanValue r;
            r.v = this->v & other.v;
            return r;
        }
        BooleanValue operator-() const {
            BooleanValue r;
            r.v = ~this->v;
            return r;
        }
        void add(bool b)
        {
            v.pushBack(b);
        }
        
        void add(BooleanValue b)
        {
            v.append(b.v);
        }
        
        std::vector<BooleanValue> get(int N)
        {
            std::vector<BooleanValue> bs;
            for(int i = 0;i<N;i++)
            {
                BooleanValue b;
                int batchSize = this->v.size() / N;
                b.v.copy(this->v,i*batchSize,batchSize);
                bs.push_back(b);
            }
            return bs;
        }

        static BooleanValue sample(osuCrypto::PRNG* prng, osuCrypto::u64 count)
        {
            BooleanValue r(count);
            r.v.randomize(*prng);
            return r;
        }
    };

    class RingValue : public DomainValue  {
        public:
        std::vector<osuCrypto::u64> v;

        RingValue()
        {
            
        }

        RingValue(osuCrypto::u64 s)
        {
            v.resize(s);
        }
        
        void print(){
            for(const auto& vv : v)
            std::cout << vv << " ";
        }

        static RingValue sample(osuCrypto::PRNG* prng, osuCrypto::u64 count)
        {
            RingValue r;
            for(osuCrypto::u64 i = 0;i<count;i++)
            {
                r.add(prng->get<osuCrypto::u64>());
            }
            return r;
        }

        RingValue operator+(const RingValue& other) const {
            RingValue r(this->v.size());
            for(int i = 0;i<v.size();i++)
            {
                r.v[i] = this->v[i] + other.v[i];
            }
            return r;
        }

        RingValue addConst(int c) {
            RingValue r(this->v.size());
            for(int i = 0;i<v.size();i++)
            {
                r.v[i] = this->v[i] + c;
            }
            return r;
        }

        RingValue multConst(int c) {
            RingValue r(this->v.size());
            for(int i = 0;i<v.size();i++)
            {
                r.v[i] = c*this->v[i];
            }
            return r;
        }


        RingValue operator-(const RingValue& other) const {
            RingValue r(this->v.size());
            for(int i = 0;i<v.size();i++)
            {
                r.v[i] = (this->v[i] - other.v[i]);
            }
            return r;
        }
        
        RingValue operator*(const RingValue& other) const {
            RingValue r(this->v.size());
            for(int i = 0;i<v.size();i++)
            {
                r.v[i] = this->v[i] * other.v[i];
            }
            return r;
        }
        
        RingValue operator-() const {
            RingValue r(this->v.size());
            for(int i = 0;i<v.size();i++)
            {
                r.v.push_back(- this->v[i]);
            }
            return r;
        }

        
        void add(osuCrypto::u64 b)
        {
            v.push_back(b);
        }
        
        void add(RingValue r)
        {
            v.insert(v.end(),r.v.begin(),r.v.end());
        }
        
        std::vector<RingValue> get(int N)
        {
            std::vector<RingValue> rs;
            int batchSize = this->v.size() / N;
            for(int i = 0;i<N;i++)
            {
                RingValue r;
                for(int j = 0;j<batchSize;j++)
                {
                    r.add(this->v[i*batchSize+j]);
                }
                rs.push_back(r);
            }
            return rs;
        }
    };

}