// This file is part of LatNet Builder.
//
// Copyright (C) 2012-2018  Pierre L'Ecuyer and Universite de Montreal
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

/**
 * \file
 * This file contains the base classes for digital nets in base 2.
 * 
 * The high-level abstract representation of a digital net essentially corresponds to a vector of generating matrices.
 * This representation is used to reason about digital nets whenever we do not need to actually construct them,
 * e.g. to implement figures of merit that run computations based on the matrices.
 * The AbstractDigitalNet class implements this representation.
 * 
 * Concrete implementations of digital nets rely on construction methods.
 * A construction method is a way of specifying the subspace of nets which is of interest for us and how to construct nets from this subspace.
 * We can be interested in exploring the whole space (which we called explicit construction, as we directly work on the generating matrices).
 * But we can also be interested in exploring specific subspaces (such as Sobol nets, polynomial lattice rules),
 * or even subspaces generated by applying randomizations to a base net.
 * 
 * A construction method is defined by choosing two main variables:
 *    + the list of generating values are used to create each net of our subspace. 
 *      There is one generating value for each coordinate of the net. 
 *      Generating values are *specific* to each net, and these are the variables which get optimized when exploring the subspace.
 *    + the size parameters are the parameters *common* to all nets which are needed by the construction rule.
 *      They are not optimized when exploring the subspace.
 *
 * There are currently 4 construction methods implemented: Sobol, polynomial, explicit, and Left Matrix Scramble
 *    + Sobol: the size parameter is the number of columns of the matrices, and the generating values are the direction numbers used to construct a given coordinate. 
 *    + Polynomial: the size parameter is the Polynomial modulus, and the generating value is the Polynomial used to construct a given coordinate.
 *    + Explicit: the size parameter is the dimensions of the matrices, and the generating value is the matrix itself.
 *    + Left Matrix Scramble: the size parameter is the base net, and the generating value is the scrambling matrix.
 * 
 * The DigitalNet class derives from AbstractDigitalNet, and contains methods common to all construction methods (such as extending the net by using a new generating value).
 * It is templated by the NetConstruction class.
 * This is the good level of abstraction in order to create tasks (e.g. search methods) on digital nets.
 * 
 * Finally, the NetConstructionTraits (in NetConstructionsTraits.h) enumerations contain, for each construction method, the functions that are specific to the construction method, the most important ones being:
 *    + how to sample a random generating value.
 *    + how to construct a generating matrix from a generating value.
 *    + (in some cases) how to explore exhaustively the space of generating values.
 * 
 * This organization allows static polymorphism, while limiting the complexity of the code (in most instances, the AbstractDigitalNet class is the good level of abstraction, and it does not require templating).
 * 
 */ 

#ifndef NETBUILDER__DIGITAL_NET_H
#define NETBUILDER__DIGITAL_NET_H

#include "latbuilder/Util.h"

#include "netbuilder/Types.h"
#include "netbuilder/GeneratingMatrix.h"
#include "netbuilder/NetConstructionTraits.h"

#include <memory>
#include <sstream>
#include <stdexcept>

namespace NetBuilder {

/** Abstract class representing digital nets in base 2. 
 * 
 * Digital nets in other bases are not implemented.
 * An abstract digital net essentially corresponds to a vector of generating matrices.
 * This class is used to reason about digital nets whenever we do not need to actually construct them, e.g. to compute
 * figures of merit from the matrices.
 * 
 * Concrete instantiations of this class are done through its derived class called DigitalNet.
*/
class AbstractDigitalNet
{
    public:

        /** 
         * Default constructor. 
         * @param dimension Number of coordinates in the net.
         * @param nRows Number of rows of the generating matrices.
         * @param nCols Number of columns of the generating matrices.
        */
        AbstractDigitalNet(Dimension dimension=0, unsigned int nRows=0, unsigned int nCols=0):
            m_dimension(dimension),
            m_nRows(nRows),
            m_nCols(nCols)
        {};

        /** 
         * Default destructor.
         */
        virtual ~AbstractDigitalNet() = default;

        /** 
         * Returns the number of columns of the generating matrices. 
         */
        unsigned int numColumns() const { return m_nCols; }

        /** 
         * Returns the number of rows of the generating matrices. 
         */
        unsigned int numRows() const { return m_nRows; };

        /** 
         * Returns the number of points of the net 
         */
        uInteger numPoints() const { return LatBuilder::intPow(2, m_nCols) ; }

        /** 
         * Returns the number of points of the net 
         */
        uInteger size() const { return LatBuilder::intPow(2, m_nCols) ; }

        /** 
         * Returns the dimension (number of coordinates) of the net.
         */
        Dimension dimension() const { return m_dimension ; }

        /** 
         * Returns the generating matrix corresponding to coordinate \c coord.
         * @param coord A coordinate (between 0 and dimension() - 1 ).
         */
        const GeneratingMatrix& generatingMatrix(Dimension coord) const 
        {
            return *m_generatingMatrices[coord];
        }

        /**
         * Formats the net for output.
         * @param outputFormat Format of output.
         * @param interlacingFactor Interlacing factor of the net.
         * @param outputStyle Specific Format of output Machine format.
         */ 
        virtual std::string format(OutputStyle outputStyle = OutputStyle::TERMINAL, unsigned int interlacingFactor = 1) const = 0;

        /** 
         * Returns a bool indicating whether the net can be viewed as a digital sequence.
         */ 
        virtual bool isSequenceViewable() const = 0;

    protected:

        Dimension m_dimension; // dimension of the net
        unsigned int m_nRows; // number of rows in generating matrices
        unsigned int m_nCols; // number of columns in generating matrices
        mutable std::vector<std::shared_ptr<GeneratingMatrix>> m_generatingMatrices; // vector of shared pointers to the generating matrices
        // The generating matrix class is defined in GeneratingMatrix.h.

        /** 
         * Most general constructor. Designed to be used by derived classes. 
         * @param dimension Dimension of the net.
         * @param nRows Number of rows of the generating matrices.
         * @param nCols Number of columns of the generating matrices.
         * @param genMatrices Vector of shared pointers to the generating matrices.
         */
        AbstractDigitalNet(Dimension dimension, unsigned int nRows, unsigned int nCols, std::vector<std::shared_ptr<GeneratingMatrix>> genMatrices):
            m_dimension(dimension),
            m_nRows(nRows),
            m_nCols(nCols),
            m_generatingMatrices(genMatrices)
        {};

};

/** Derived class of AbstractDigitalNet designed to implement specific construction methods. The available construction methods
 * are described by the NetConstruction enumeration which is a non-type template parameter of the DigitalNet 
 * class. Construction methods are based on two parameters: a size parameter which is 
 * shared by all coordinates and a sequence of generating values, each of them being associated to one coordinate.
 * It is templated by a NetConstructionTraits, which describes the construction method.
 * @see NetBuilder::NetConstructionTraits
 */ 
template <NetConstruction NC>
class DigitalNet : public AbstractDigitalNet
{
    public:

        typedef NetConstructionTraits<NC> ConstructionMethod;

        /// Type of the generating values of the method. The generating value is the parameter needed to construct a 
        // generating matrix for a given coordinate. When optimizing a FOM, the space of generating values is explored,
        // and the best generating value is returned.
        typedef typename ConstructionMethod::GenValue GenValue;

        /// Type of the size parameter of the method. The size parameters are the parameters needed to construct
        // the net, which are common to all nets in the subspace of interest. Hence they are *not* optimized when
        // exploring the space of generating values.
        typedef typename ConstructionMethod::SizeParameter SizeParameter;


        /** 
         * Construction of a net from its size parameter and generating values.
         * This operation computes and stores the generating matrices of the net.
         * @param dimension Dimension of the net.
         * @param sizeParameter Size parameter of the net.
         * @param genValues Sequence of generating values to create the net.
         */
        DigitalNet(
            Dimension dimension,
            SizeParameter sizeParameter, 
            std::vector<GenValue> genValues):
                AbstractDigitalNet(dimension, ConstructionMethod::nRows(sizeParameter), ConstructionMethod::nCols(sizeParameter)),
                m_sizeParameter(std::move(sizeParameter))
        {
            m_genValues.reserve(m_dimension);
            Dimension dimension_j = 0; //index of the dimension of the net, used for creating JoeKuo net 
            for(const auto& genValue : genValues)
            {
                // construct the generating matrix and store them and the generating values
                m_generatingMatrices.push_back(std::shared_ptr<GeneratingMatrix>(ConstructionMethod::createGeneratingMatrix(genValue,m_sizeParameter,dimension_j)));
                m_genValues.push_back(std::shared_ptr<GenValue>(new GenValue(std::move(genValue))));
                dimension_j++;
            }
        }

        /** 
         * Dummy constructor for placeholder nets.
         * @param dimension Dimension of the net.
         * @param sizeParameter Size parameter of the net.
         */
        DigitalNet(
            Dimension dimension = 0,
            SizeParameter sizeParameter = SizeParameter()):
                AbstractDigitalNet(dimension, ConstructionMethod::nRows(sizeParameter), ConstructionMethod::nCols(sizeParameter)),
                m_sizeParameter(std::move(sizeParameter))
        {}

        /**
         * Default destructor
         */
        ~DigitalNet() = default;

        /** Adds a new coordinate at the end of a digital net using the generating value \c newGenValue. 
         * Note that the resources (generating matrices, generatins values and computation data) for the lower dimensions are not copied. The net on 
         * which this method is called and the new net share these resources.
         * @param newGenValue  Generating value used to extend the net.
         * @return A <code>std::unique_ptr</code> to the instantiated net.
         */ 
        std::unique_ptr<DigitalNet<NC>> appendNewCoordinate(const GenValue& newGenValue) const 
        {
            std::shared_ptr<GeneratingMatrix> newMat(ConstructionMethod::createGeneratingMatrix(newGenValue, m_sizeParameter, m_dimension));

            // copy the vector of pointers to matrices and add the new matrix
            auto genMats = m_generatingMatrices; 
            genMats.push_back(std::move(newMat));

            // copy the vector of pointers to generating values and add the new generating value
            auto genVals = m_genValues;
            genVals.push_back(std::shared_ptr<GenValue>(new GenValue(newGenValue)));

            // instantiate the new net and return the unique pointer to it
            return std::unique_ptr<DigitalNet<NC>>(new DigitalNet<NC>(m_dimension+1, m_sizeParameter, std::move(genVals), std::move(genMats)));
        }


        /**
         * {@inheritDoc}
         */ 
        virtual std::string format(OutputStyle outputStyle = OutputStyle::TERMINAL, unsigned int interlacingFactor = 1) const
        {   
            std::string res;

            if (outputStyle == OutputStyle::TERMINAL){
                std::ostringstream stream;
                stream << numColumns() << "  // Number of columns" << std::endl;
                stream << numRows() << "  // Number of rows" << std::endl;
                stream << numPoints() << "  // Number of points" << std::endl;
                stream << dimension() / interlacingFactor << "  // Dimension of points" << std::endl;
                if (interlacingFactor > 1){
                    stream << interlacingFactor << "  // Interlacing factor" << std::endl;
                    stream << dimension() << "  // Number of components = interlacing factor x dimension" << std::endl;
                }
                res+=stream.str();
            }

            else if (outputStyle == OutputStyle::NET) {
                std::string dimension_as_str = std::to_string(dimension());
                std::string nb_col_as_str = std::to_string(numColumns());
                res += "# Parameters for a digital net in base 2\n";
                res += dimension_as_str + "    # " + dimension_as_str + " dimensions\n";
                if (interlacingFactor > 1){
                    res += std::to_string(interlacingFactor) + "  // Interlacing factor" + "\n";
                    res += std::to_string(dimension()) + "  // Number of components = interlacing factor x dimension" + "\n";
                }
                res += nb_col_as_str + "   # k = " + nb_col_as_str + ",  n = 2^"+ nb_col_as_str + " = "; 
                res += std::to_string(numPoints()) + " points\n";
                res += "31   # r = 31 binary output digits\n";
                if (interlacingFactor == 1){
                    res += "# Columns of gen. matrices C_1,...,C_s, one matrix per line:\n";
                }
                else {
                    res += "# Columns of gen. matrices C_1,...,C_{ds}, one matrix per line:\n";
                }
                for(unsigned int coord = 0; coord < m_genValues.size(); coord++)
                {
                    res += m_generatingMatrices[coord]->formatToColumnsReverse();
                    res += "\n";
                }
                res.pop_back();
            }

            res += ConstructionMethod::format(m_generatingMatrices, m_genValues, m_sizeParameter, outputStyle, interlacingFactor);

            return res;
        }

        /**
         * {@inheritDoc}
         */ 
        virtual bool isSequenceViewable() const 
        {
            return ConstructionMethod::isSequenceViewable;
        }

        SizeParameter sizeParameter() const { return m_sizeParameter ; }
    
    private:

        SizeParameter m_sizeParameter; // size parameter of the net
        std::vector<std::shared_ptr<GenValue>> m_genValues; // vector of shared pointers to the generating values of the net

        /** Constructor used internally to avoid recomputing known generating matrices.
         * @param dimension Dimension of the net.
         * @param sizeParameter Size parameter of the net.
         * @param genValues Vector of shared pointers to the generating values of each coordinate.
         * @param genMatrices Vector of shared pointers to the generating matrices of each coordinate.
        */ 
        DigitalNet(
            Dimension dimension,
            SizeParameter sizeParameter,
            std::vector<std::shared_ptr<GenValue>> genValues,
            std::vector<std::shared_ptr<GeneratingMatrix>> genMatrices
            ):
                AbstractDigitalNet(dimension, ConstructionMethod::nRows(sizeParameter), ConstructionMethod::nCols(sizeParameter), std::move(genMatrices)),
                m_sizeParameter(std::move(sizeParameter)),
                m_genValues(std::move(genValues))
        {};
};
}

#endif
