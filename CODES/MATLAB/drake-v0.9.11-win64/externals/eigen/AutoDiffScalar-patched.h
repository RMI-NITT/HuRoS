// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_AUTODIFF_SCALAR_H
#define EIGEN_AUTODIFF_SCALAR_H

namespace Eigen {

namespace internal {

template<typename A, typename B>
struct make_coherent_impl {
  static void run(A&, B&) {}
};

// resize a to match b is a.size()==0, and conversely.
template<typename A, typename B>
void make_coherent(const A& a, const B&b)
{
  make_coherent_impl<A,B>::run(a.const_cast_derived(), b.const_cast_derived());
}

template<typename _DerType, bool Enable> struct auto_diff_special_op;

} // end namespace internal

/** \class AutoDiffScalar
  * \brief A scalar type replacement with automatic differentation capability
  *
  * \param _DerType the vector type used to store/represent the derivatives. The base scalar type
  *                 as well as the number of derivatives to compute are determined from this type.
  *                 Typical choices include, e.g., \c Vector4f for 4 derivatives, or \c VectorXf
  *                 if the number of derivatives is not known at compile time, and/or, the number
  *                 of derivatives is large.
  *                 Note that _DerType can also be a reference (e.g., \c VectorXf&) to wrap a
  *                 existing vector into an AutoDiffScalar.
  *                 Finally, _DerType can also be any Eigen compatible expression.
  *
  * This class represents a scalar value while tracking its respective derivatives using Eigen's expression
  * template mechanism.
  *
  * It supports the following list of global math function:
  *  - std::abs, std::sqrt, std::pow, std::exp, std::log, std::sin, std::cos,
  *  - internal::abs, internal::sqrt, numext::pow, internal::exp, internal::log, internal::sin, internal::cos,
  *  - internal::conj, internal::real, internal::imag, numext::abs2.
  *
  * AutoDiffScalar can be used as the scalar type of an Eigen::Matrix object. However,
  * in that case, the expression template mechanism only occurs at the top Matrix level,
  * while derivatives are computed right away.
  *
  */

template<typename _DerType>
class AutoDiffScalar
  : public internal::auto_diff_special_op
            <_DerType, !internal::is_same<typename internal::traits<typename internal::remove_all<_DerType>::type>::Scalar,
                                        typename NumTraits<typename internal::traits<typename internal::remove_all<_DerType>::type>::Scalar>::Real>::value>
{
  public:
    typedef internal::auto_diff_special_op
            <_DerType, !internal::is_same<typename internal::traits<typename internal::remove_all<_DerType>::type>::Scalar,
                       typename NumTraits<typename internal::traits<typename internal::remove_all<_DerType>::type>::Scalar>::Real>::value> Base;
    typedef typename internal::remove_all<_DerType>::type DerType;
    typedef typename internal::traits<DerType>::Scalar Scalar;
    typedef typename NumTraits<Scalar>::Real Real;

    using Base::operator+;
    using Base::operator*;

    /** Default constructor without any initialization. */
    AutoDiffScalar() {}

    /** Constructs an active scalar from its \a value,
        and initializes the \a nbDer derivatives such that it corresponds to the \a derNumber -th variable */
    AutoDiffScalar(const Scalar& value, int nbDer, int derNumber)
      : m_value(value), m_derivatives(DerType::Zero(nbDer))
    {
      m_derivatives.coeffRef(derNumber) = Scalar(1);
    }

    /** Conversion from a scalar constant to an active scalar.
      * The derivatives are set to zero. */
    /*explicit*/ AutoDiffScalar(const Real& value)
      : m_value(value)
    {
      if(m_derivatives.size()>0)
        m_derivatives.setZero();
    }

    /** Constructs an active scalar from its \a value and derivatives \a der */
    AutoDiffScalar(const Scalar& value, const DerType& der)
      : m_value(value), m_derivatives(der)
    {}

    template<typename OtherDerType>
    AutoDiffScalar(const AutoDiffScalar<OtherDerType>& other)
      : m_value(other.value()), m_derivatives(other.derivatives())
    {}

    friend  std::ostream & operator << (std::ostream & s, const AutoDiffScalar& a)
    {
      return s << a.value();
    }

    AutoDiffScalar(const AutoDiffScalar& other)
      : m_value(other.value()), m_derivatives(other.derivatives())
    {}

    template<typename OtherDerType>
    inline AutoDiffScalar& operator=(const AutoDiffScalar<OtherDerType>& other)
    {
      m_value = other.value();
      m_derivatives = other.derivatives();
      return *this;
    }

    inline AutoDiffScalar& operator=(const AutoDiffScalar& other)
    {
      m_value = other.value();
      m_derivatives = other.derivatives();
      return *this;
    }

//     inline operator const Scalar& () const { return m_value; }
//     inline operator Scalar& () { return m_value; }

    inline const Scalar& value() const { return m_value; }
    inline Scalar& value() { return m_value; }

    inline const DerType& derivatives() const { return m_derivatives; }
    inline DerType& derivatives() { return m_derivatives; }

    inline bool operator< (const Scalar& other) const  { return m_value <  other; }
    inline bool operator<=(const Scalar& other) const  { return m_value <= other; }
    inline bool operator> (const Scalar& other) const  { return m_value >  other; }
    inline bool operator>=(const Scalar& other) const  { return m_value >= other; }
    inline bool operator==(const Scalar& other) const  { return m_value == other; }
    inline bool operator!=(const Scalar& other) const  { return m_value != other; }

    friend inline bool operator< (const Scalar& a, const AutoDiffScalar& b) { return a <  b.value(); }
    friend inline bool operator<=(const Scalar& a, const AutoDiffScalar& b) { return a <= b.value(); }
    friend inline bool operator> (const Scalar& a, const AutoDiffScalar& b) { return a >  b.value(); }
    friend inline bool operator>=(const Scalar& a, const AutoDiffScalar& b) { return a >= b.value(); }
    friend inline bool operator==(const Scalar& a, const AutoDiffScalar& b) { return a == b.value(); }
    friend inline bool operator!=(const Scalar& a, const AutoDiffScalar& b) { return a != b.value(); }

    template<typename OtherDerType> inline bool operator< (const AutoDiffScalar<OtherDerType>& b) const  { return m_value <  b.value(); }
    template<typename OtherDerType> inline bool operator<=(const AutoDiffScalar<OtherDerType>& b) const  { return m_value <= b.value(); }
    template<typename OtherDerType> inline bool operator> (const AutoDiffScalar<OtherDerType>& b) const  { return m_value >  b.value(); }
    template<typename OtherDerType> inline bool operator>=(const AutoDiffScalar<OtherDerType>& b) const  { return m_value >= b.value(); }
    template<typename OtherDerType> inline bool operator==(const AutoDiffScalar<OtherDerType>& b) const  { return m_value == b.value(); }
    template<typename OtherDerType> inline bool operator!=(const AutoDiffScalar<OtherDerType>& b) const  { return m_value != b.value(); }

    inline const AutoDiffScalar<DerType&> operator+(const Scalar& other) const
    {
      return AutoDiffScalar<DerType&>(m_value + other, m_derivatives);
    }

    friend inline const AutoDiffScalar<DerType&> operator+(const Scalar& a, const AutoDiffScalar& b)
    {
      return AutoDiffScalar<DerType&>(a + b.value(), b.derivatives());
    }

//     inline const AutoDiffScalar<DerType&> operator+(const Real& other) const
//     {
//       return AutoDiffScalar<DerType&>(m_value + other, m_derivatives);
//     }

//     friend inline const AutoDiffScalar<DerType&> operator+(const Real& a, const AutoDiffScalar& b)
//     {
//       return AutoDiffScalar<DerType&>(a + b.value(), b.derivatives());
//     }

    inline AutoDiffScalar& operator+=(const Scalar& other)
    {
      value() += other;
      return *this;
    }

    template<typename OtherDerType>
    inline const AutoDiffScalar<CwiseBinaryOp<internal::scalar_sum_op<Scalar>,const DerType,const typename internal::remove_all<OtherDerType>::type> >
    operator+(const AutoDiffScalar<OtherDerType>& other) const
    {
      internal::make_coherent(m_derivatives, other.derivatives());
      return AutoDiffScalar<CwiseBinaryOp<internal::scalar_sum_op<Scalar>,const DerType,const typename internal::remove_all<OtherDerType>::type> >(
        m_value + other.value(),
        m_derivatives + other.derivatives());
    }

    template<typename OtherDerType>
    inline AutoDiffScalar&
    operator+=(const AutoDiffScalar<OtherDerType>& other)
    {
      (*this) = (*this) + other;
      return *this;
    }

    inline const AutoDiffScalar<DerType&> operator-(const Scalar& b) const
    {
      return AutoDiffScalar<DerType&>(m_value - b, m_derivatives);
    }

    friend inline const AutoDiffScalar<CwiseUnaryOp<internal::scalar_opposite_op<Scalar>, const DerType> >
    operator-(const Scalar& a, const AutoDiffScalar& b)
    {
      return AutoDiffScalar<CwiseUnaryOp<internal::scalar_opposite_op<Scalar>, const DerType> >
            (a - b.value(), -b.derivatives());
    }

    inline AutoDiffScalar& operator-=(const Scalar& other)
    {
      value() -= other;
      return *this;
    }

    template<typename OtherDerType>
    inline const AutoDiffScalar<CwiseBinaryOp<internal::scalar_difference_op<Scalar>, const DerType,const typename internal::remove_all<OtherDerType>::type> >
    operator-(const AutoDiffScalar<OtherDerType>& other) const
    {
      internal::make_coherent(m_derivatives, other.derivatives());
      return AutoDiffScalar<CwiseBinaryOp<internal::scalar_difference_op<Scalar>, const DerType,const typename internal::remove_all<OtherDerType>::type> >(
        m_value - other.value(),
        m_derivatives - other.derivatives());
    }

    template<typename OtherDerType>
    inline AutoDiffScalar&
    operator-=(const AutoDiffScalar<OtherDerType>& other)
    {
      *this = *this - other;
      return *this;
    }

    inline const AutoDiffScalar<CwiseUnaryOp<internal::scalar_opposite_op<Scalar>, const DerType> >
    operator-() const
    {
      return AutoDiffScalar<CwiseUnaryOp<internal::scalar_opposite_op<Scalar>, const DerType> >(
        -m_value,
        -m_derivatives);
    }

    inline const AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const DerType> >
    operator*(const Scalar& other) const
    {
      return AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const DerType> >(
        m_value * other,
        (m_derivatives * other));
    }

    friend inline const AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const DerType> >
    operator*(const Scalar& other, const AutoDiffScalar& a)
    {
      return AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const DerType> >(
        a.value() * other,
        a.derivatives() * other);
    }

//     inline const AutoDiffScalar<typename CwiseUnaryOp<internal::scalar_multiple_op<Real>, DerType>::Type >
//     operator*(const Real& other) const
//     {
//       return AutoDiffScalar<typename CwiseUnaryOp<internal::scalar_multiple_op<Real>, DerType>::Type >(
//         m_value * other,
//         (m_derivatives * other));
//     }
//
//     friend inline const AutoDiffScalar<typename CwiseUnaryOp<internal::scalar_multiple_op<Real>, DerType>::Type >
//     operator*(const Real& other, const AutoDiffScalar& a)
//     {
//       return AutoDiffScalar<typename CwiseUnaryOp<internal::scalar_multiple_op<Real>, DerType>::Type >(
//         a.value() * other,
//         a.derivatives() * other);
//     }

    inline const AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const DerType> >
    operator/(const Scalar& other) const
    {
      return AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const DerType> >(
        m_value / other,
        (m_derivatives * (Scalar(1)/other)));
    }

    friend inline const AutoDiffScalar<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         