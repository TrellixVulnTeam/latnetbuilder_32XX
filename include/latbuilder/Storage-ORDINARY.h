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

#ifndef LATBUILDER__STORAGE_ORDINARY_H
#define LATBUILDER__STORAGE_ORDINARY_H

#include "latbuilder/Storage.h"
#include "latbuilder/CompressTraits.h"
#include "latbuilder/SizeParam.h"

namespace LatBuilder {



template <Lattice LR, Compress COMPRESS>
struct StorageTraits<Storage<LR, LatType::ORDINARY, COMPRESS>> {
   typedef uInteger            size_type;
   typedef typename LatticeTraits<LR>::GenValue  value_type;
   typedef CompressTraits<COMPRESS> Compress;
   typedef Real               MeritValue;
   typedef LatBuilder::SizeParam<LR, LatType::ORDINARY> SizeParam;

   /**
    * Unpermuted permutation.
    */
   class Unpermute {
   public:
      typedef StorageTraits::size_type size_type;

      Unpermute(Storage<LR, LatType::ORDINARY, COMPRESS> storage): m_storage(std::move(storage)) {}
      size_type operator() (size_type i) const { return Compress::compressIndex(i, m_storage.virtualSize()); }
      size_type size() const { return m_storage.virtualSize(); }
   private:
      Storage<LR, LatType::ORDINARY, COMPRESS> m_storage;
   };

   /**
    * Stride permutation.
    *
    * Depending on the lattice: 
    *
    * \paragraph 
    *
    * For integration lattices: \n
    * a Stride with parameter \f$a \in \mathbb{N}\f$ is the mapping that maps an index \f$i\f$ to \f$a \times i \mod n\f$.
    *
    * Consider the unpermuted vector \f$\boldsymbol v = (v_0, \dots, v_{n-1})\f$ for
    * some positive integer \f$n\f$.  The \f$j\f$-th component of the vector
    * with stride length \f$a\f$ has value \f$v_{j a \bmod n}\f$.
    *
    * \paragraph 
    *
    * For polynomial lattices: \n
    * A Stride with parameter \f$q(z) \in \mathbb{Z}_2[z]\f$ is the mapping that maps a polynomial \f$i(z)\f$ modulo \f$P(z)\f$ with the polynomial \f$h(z) = i(z)q(z) \mod P(z)\f$ .
    * 
    * Consider the unpermuted vector \f$\boldsymbol v = (v_0, \dots, v_{n-1}) \sim (v_{0(z)}, \dots, v_{(n-1)(z)})\f$ for
    * some positive integer \f$n\f$, where \f$j(z) = \sum a_lz^l\f$ if \f$j = \sum a_l2^l\f$. Let \f$P(z)\f$ be the modulus polynomial.  The \f$j\f$-th component of the vector
    * with stride parameter \f$q(z) \in \mathbb{Z}_2[z]\f$ has value \f$v_{j(z) q(z) \bmod P(z)}\f$.
    */
   class Stride {
   public:
      typedef StorageTraits::size_type size_type;
      typedef StorageTraits::value_type value_type;

      Stride(Storage<LR, LatType::ORDINARY, COMPRESS> storage, value_type stride):
         m_storage(std::move(storage)),
         m_stride(stride)
      {}

      size_type operator() (size_type i) const
      {
         const auto modulus = m_storage.sizeParam().modulus();
         const auto numPoints = m_storage.sizeParam().numPoints();
         return Compress::compressIndex(LatticeTraits<LR>::ToIndex(m_stride * LatticeTraits<LR>::ToGenValue(i) % modulus), numPoints);
      }

      size_type size() const
      { return m_storage.size(); }

   private:
      Storage<LR, LatType::ORDINARY, COMPRESS> m_storage;
      value_type m_stride;
   };

};

/**
 * Storage class for flat vectors.
 *
 * The vector elements are not permuted but compression is applied to vector
 * indices.
 *
 * \tparam COMPRESS     Compression type (either None or Symmetric).
 */
template <Lattice LR, Compress COMPRESS, PerLvlOrder PLO>
class Storage<LR, LatType::ORDINARY, COMPRESS, PLO> :
   public BasicStorage<Storage<LR, LatType::ORDINARY, COMPRESS, PLO>> {

   typedef Storage<LR, LatType::ORDINARY, COMPRESS, PLO> self_type;

public:

   typedef typename self_type::size_type  size_type;
   typedef typename self_type::value_type  value_type;
   typedef typename self_type::MeritValue MeritValue;
   typedef typename self_type::SizeParam  SizeParam;

   static std::string shortname()
   { return "flat storage"; }

   Storage(SizeParam sizeParam):
      BasicStorage<Storage>(std::move(sizeParam))
   {
      if(PLO == PerLvlOrder::CYCLIC) 
        throw std::invalid_argument("Storage(): Trying to instantiate Storage<LatType::ORDINARY, PerLvlOrder::Cyclic>");
   }

   size_type virtualSize() const
   { return this->sizeParam().numPoints(); }

   MeritValue createMeritValue(Real value = 0.0) const
   { return value; }
};

}

#endif
