// This file is part of Lattice Builder.
//
// Copyright (C) 2012-2016  Pierre L'Ecuyer and Universite de Montreal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "latbuilder/DigitalNet/ComputationScheme.h"
#include <iostream>
#include <boost/dynamic_bitset.hpp>

using namespace LatBuilder::DigitalNet;


typedef boost::dynamic_bitset<> projection;
typedef LatBuilder::uInteger uInteger;

class dummyWeights{
    public:
        float operator()(const projection& projRep){ return rand() % 1000;; }
};

struct dummyMethod{
    template <uInteger BASE>
    int computeTValue(std::vector<Matrix<BASE>> matrices, int lowerBound){
        return 0;
    }
};

int main(int argc, const char *argv[])
{
    int lastDimension = 6;
    int maximalCardinality = 4;

    auto foo =  ComputationScheme<dummyWeights,dummyMethod>(lastDimension,maximalCardinality,dummyWeights());

    std::cout << foo;

    foo.setTValuesBestNet();

    auto foo2 = ComputationScheme<dummyWeights,dummyMethod>(lastDimension+1,maximalCardinality,dummyWeights(),foo);

    std::cout << foo2;

    return 0;
}
