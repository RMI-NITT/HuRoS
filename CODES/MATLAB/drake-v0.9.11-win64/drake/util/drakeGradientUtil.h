#ifndef DRAKEGRADIENTUTIL_H_
#define DRAKEGRADIENTUTIL_H_

#include <Eigen/Core>
#include <Eigen/Dense>
#include <unsupported/Eigen/AutoDiff>
#include <cmath>
#include <vector>
#include <array>
#include <cassert>
#include <stdexcept>


template<std::size_t Size>
std::array<int, Size> intRange(int start)
{
  std::array<int, Size> ret;
  for (unsigned int i = 0; i < Size; i++) {
    ret[i] = i + start;
  }
  return ret;
}

namespace Drake {
  // todo: recursive template to get arbitrary gradient order

  // note: tried using template default values (e.g. Eigen::Dynamic), but they didn't seem to work on my mac clang
  template <int num_vars> using TaylorVar = Eigen::AutoDiffScalar< Eigen::Matrix<double,num_vars,1> >;
  template <int num_vars, int rows> using TaylorVec = Eigen::Matrix< TaylorVar<num_vars>, rows, 1>;
  template <int num_vars, int rows, int cols> using TaylorMat = Eigen::Matrix< TaylorVar<num_vars>, rows, cols>;

  typedef TaylorVar<Eigen::Dynamic> TaylorVarX;
  typedef TaylorVec<Eigen::Dynamic,Eigen::Dynamic> TaylorVecX;
  typedef TaylorMat<Eigen::Dynamic,Eigen::Dynamic,Eigen::Dynamic> TaylorMatX;

  // initializes the vector with x=val and dx=eye(numel(val))
  template <typename Derived>
  TaylorVecX initTaylorVecX(const Eigen::MatrixBase<Derived>& val) {
    TaylorVecX x(val.rows());
    Eigen::MatrixXd der = Eigen::MatrixXd::Identity(val.rows(),val.rows());
    for (int i=0; i<val.rows(); i++) {
      x(i).value() = val(i);
      x(i).derivatives() = der.col(i);
    }
    return x;
  }
}

/*
 * Recursively defined template specifying a matrix type of the correct size for a gradient of a matrix function with respect to Nq variables, of any order.
 */
template<typename Derived, int Nq, int DerivativeOrder = 1>
struct Gradient {
  typedef typename Eigen::Matrix<typename Derived::Scalar, ((Derived::SizeAtCompileTime == Eigen::Dynamic || Nq == Eigen::Dynamic) ? Eigen::Dynamic : Gradient<Derived, Nq, DerivativeOrder - 1>::type::SizeAtCompileTime), Nq> type;
};

/*
 * Base case for recursively defined gradient template.
 */
template<typename Derived, int Nq>
struct Gradient<Derived, Nq, 1> {
  typedef typename Eigen::Matrix<typename Derived::Scalar, Derived::SizeAtCompileTime, Nq> type;
};

/*
 * Output type of matGradMultMat
 */
template<typename DerivedA, typename DerivedB, typename DerivedDA>
struct MatGradMultMat {
  typedef typename Eigen::Matrix<typename DerivedA::Scalar, (DerivedA::RowsAtCompileTime == Eigen::Dynamic || DerivedB::ColsAtCompileTime == Eigen::Dynamic ? Eigen::Dynamic : DerivedA::RowsAtCompileTime * DerivedB::ColsAtCompileTime), DerivedDA::ColsAtCompileTime> type;
};

/*
 * Output type of matGradMult
 */
template<typename DerivedDA, typename DerivedB>
struct MatGradMult {
  typedef typename Eigen::Matrix<typename DerivedDA::Scalar, (DerivedDA::RowsAtCompileTime == Eigen::Dynamic || DerivedB::SizeAtCompileTime == Eigen::Dynamic ? Eigen::Dynamic : DerivedDA::RowsAtCompileTime / DerivedB::RowsAtCompileTime * DerivedB::ColsAtCompileTime), DerivedDA::ColsAtCompileTime> type;
};

/*
 * Output type and array types of getSubMatrixGradient for std::arrays specifying rows and columns
 */
template<int QSubvectorSize, typename Derived, std::size_t NRows, std::size_t NCols>
struct GetSubMatrixGradientArray {
  typedef typename Eigen::Matrix<typename Derived::Scalar, (NRows * NCols), ((QSubvectorSize == Eigen::Dynamic) ? Derived::ColsAtCompileTime : QSubvectorSize)> type;
};

/*
 * Output type of getSubMatrixGradient for a single element of the input matrix
 */
template<int QSubvectorSize, typename Derived>
struct GetSubMatrixGradientSingleElement {
  typedef typename Eigen::Block<const Derived, 1, ((QSubvectorSize == Eigen::Dynamic) ? Derived::ColsAtCompileTime : QSubvectorSize)> type;
};

template <typename Derived>
struct AutoDiffToValueMatrix {
  typedef typename Eigen::Matrix<typename Derived::Scalar::Scalar, Derived::RowsAtCompileTime, Derived::ColsAtCompileTime> type;
};

template <typename Derived>
struct AutoDiffToGradientMatrix {
  typedef typename Gradient<Eigen::Matrix<typename Derived::Scalar::Scalar, Derived::RowsAtCompileTime, Derived::ColsAtCompileTime>, Eigen::Dynamic>::type type;
};

/*
 * Profile results: looks like return value optimization works; a version that sets a reference
 * instead of returning a value is just as fast and this is cleaner.
 */
template <typename Derived>
typename Derived::PlainObject transposeGrad(
    const Eigen::MatrixBase<Derived>& dX, typename Derived::Index rows_X)
{
  typename Derived::PlainObject dX_transpose(dX.rows(), dX.cols());
  typename Derived::Index numel = dX.rows();
  typename Derived::Index index = 0;
  for (int i = 0; i < numel; i++) {
    dX_transpose.row(i) = dX.row(index);
    index += rows_X;
    if (index >= numel) {
      index = (index % numel) + 1;
    }
  }
  return dX_transpose;
}

template <typename DerivedA, typename DerivedB, typename DerivedDA, typename DerivedDB>
typename MatGradMultMat<DerivedA, DerivedB, DerivedDA>::type
matGradMultMat(
    const Eigen::MatrixBase<DerivedA>& A,
    const Eigen::MatrixBase<DerivedB>& B,
    const Eigen::MatrixBase<DerivedDA>& dA,
    const Eigen::MatrixBase<DerivedDB>& dB) {
  assert(dA.cols() == dB.cols());

  typename MatGradMultMat<DerivedA, DerivedB, DerivedDA>::type ret(A.rows() * B.cols(), dA.cols());

  for (int col = 0; col < B.cols(); col++) {
    auto block = ret.template block<DerivedA::RowsAtCompileTime, DerivedDA::ColsAtCompileTime>(col * A.rows(), 0, A.rows(), dA.cols());

    // A * dB part:
    block.noalias() = A * dB.template block<DerivedA::ColsAtCompileTime, DerivedDA::ColsAtCompileTime>(col * A.cols(), 0, A.cols(), dA.cols());

    for (int row = 0; row < B.rows(); row++) {
      // B * dA part:
      block.noalias() += B(row, col) * dA.template block<DerivedA::RowsAtCompileTime, DerivedDA::ColsAtCompileTime>(row * A.rows(), 0, A.rows(), dA.cols());
    }
  }
  return ret;

  // much slower and requires eigen/unsupported:
//  return Eigen::kroneckerProduct(Eigen::MatrixXd::Identity(B.cols(), B.cols()), A) * dB + Eigen::kroneckerProduct(B.transpose(), Eigen::MatrixXd::Identity(A.rows(), A.rows())) * dA;
}

template<typename DerivedDA, typename DerivedB>
typename MatGradMult<DerivedDA, DerivedB>::type
matGradMult(const Eigen::MatrixBase<DerivedDA>& dA, const Eigen::MatrixBase<DerivedB>& B) {
  assert(B.rows() == 0 ? dA.rows() == 0 : dA.rows() % B.rows() == 0);
  typename DerivedDA::Index A_rows = B.rows() == 0 ? 0 : dA.rows() / B.rows();
  const int A_rows_at_compile_time = (DerivedDA::RowsAtCompileTime == Eigen::Dynamic || DerivedB::RowsAtCompileTime == Eigen::Dynamic) ?
                                     Eigen::Dynamic :
                                     static_cast<int>(DerivedDA::RowsAtCompileTime / DerivedB::RowsAtCompileTime);

  typename MatGradMult<DerivedDA, DerivedB>::type ret(A_rows * B.cols(), dA.cols());
  ret.setZero();
  for (int col = 0; col < B.cols(); col++) {
    auto block = ret.template block<A_rows_at_compile_time, DerivedDA::ColsAtCompileTime>(col * A_rows, 0, A_rows, dA.cols());
    for (int row = 0; row < B.rows(); row++) {
      // B * dA part:
      block.noalias() += B(row, col) * dA.template block<A_rows_at_compile_time, DerivedDA::ColsAtCompileTime>(row * A_rows, 0, A_rows, dA.cols());
    }
  }
  return ret;
}

// TODO: could save copies once http://eigen.tuxfamily.org/bz/show_bug.cgi?id=329 is fixed
template<typename Derived>
Eigen::Matrix<typename Derived::Scalar, Eigen::Dynamic, Eigen::Dynamic> getSubMatrixGradient(
    const Eigen::MatrixBase<Derived>& dM, const std::vector<int>& rows, const std::vector<int>& cols,
    typename Derived::Index M_rows, int q_start = 0, typename Derived::Index q_subvector_size = -1) {
  if (q_subvector_size < 0) {
    q_subvector_size = dM.cols() - q_start;
  }
  Eigen::MatrixXd dM_submatrix(rows.size() * cols.size(), q_subvector_size);
  int index = 0;
  for (std::vector<int>::const_iterator col = cols.begin(); col != cols.end(); ++col) {
    for (std::vector<int>::const_iterator row = rows.begin(); row != rows.end(); ++row) {
      dM_submatrix.row(index) = dM.block(*row + *col * M_rows, q_start, 1, q_subvector_size);
 ?????ÿÿÿÿÿÿÿÿÿÿÿÿÿˆ² Üh|  ÿ‘TÇ[  - <   <  b ad alloc ation  R SDSñßPå ‚1N¢pX´a' ^win dowscode csext.pdb ,‹ÿU‹ì ‹E÷ØÀ% Á @  €]Â jÿÿ@ pqÌÄ{ w7ÚÌ¾ ExitP rocess K ERNEL32.dll	-  (¡ß6  ¯ U€$€0€ŠU€)“€p€€K€o€‰ WƒpC‚pE€p1IWI CColorTr ansform_ Initialiˆze_ Kxy `reateƒD llGetCla ssObjectÿ‹[ÿ ÿ??????ÿ????????ÿ????????ÿ???????ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ°  pqù?                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  °  pq¹;,ğ»ş  ÿÿÿÿÿÿ  –³" 
   x $ €
°    ‚0\	  H XÀ`@  ôjô4 V S €_ V E R  I O N I 
F '½ïş g jD±* ?   X  	R o Ot r i n g F l eD I f o ,.#0 4 0 9 4 B   L  mC #m p a€y€Fa m+€1M€$c€+o s€f t   Ur€o€a€iÅ€n | *€%…=RD€ s i€t-€#o€.•)W€n Zd€$w€4ƒ1d€ cÑ‚E x€3e€	*d€AL€b‚?r ¢y * 7ˆ=V€r€…L  6 .T 1€7€0‚1U€5€4€&(€/pUÀp@cs@+2Àr-@m@ÃL8@K3 - 2@5 8 )@ Ô Á8B «À#Á_tÂ"nÀVlHV_Ë;ATE;Ã:Á€@´ LÀgBA	p@;Ayg hÀ'  ©‹À çh.@A l@ Z ÀNi@ŠAs@rE@sÀ r v@dÑ@  JÂ0OD
A–>a@ÅwA4C‰á2j ª%ÀCPÀo@yuÀ…ºtÜ™®@%KAO@g®e@ÃÁº @¾yÀníÁYmM@¶ÎM„ÙƒÕD€  ua`%§i  $àp  T5"n l 4i\	ş° ‚!	 ????ÿ????????ÿ????????ÿ???????ÿÿÿÿÿÿÿÿÿÿÿÿÿÿº        0—0     `000 0000 0 0$0(0 ,0004080 <0@0D0H0 L0P0T0X0 \0`0d0h0 l0p0t0x0 |0€0„0ˆ0 Œ00”0˜0 œ0 0¤0¨0 ¬0°0´0¸0 ¼0À0Ä0È0 Ì0Ğ0Ô0Ø0 Ü0à0ä0è0 ì0ğ0ô0ø0 ü0 111 1111 1 1$1(1 ,1014181 <1@1D1H1 L1P1T1X1 \1`1d1h1 l1p1t1x1 |1€1„1ˆ1 Œ11”1˜1 œ1 1¤1¨1 ¬1°1´1¸1 ¼1À1Ä1È1 Ì1Ğ1Ô1Ø1 Ü1à1ä1è1 ì1ğ1ô1ø1 ü1 222 2222 2 2$2(2 ,2024282 <2@2D2H2 L2P2T2X2 \2`2d2h2 l2p2t2x2 |2€2„2ˆ2 Œ22”2˜2 œ2 2¤2¨2 ¬2°2´2¸2 ¼2À2Ä2È2 Ì2Ğ2Ô2Ø2 Ü2à2ä2è2 ì2ğ2ô2ø2 ü2 333 3333 3 3$3(3 ,3034383 <3@3D3H3 L3P3T3X3 \3`3d3h3 l3p3t3x3 |3€3„3ˆ3 Œ33”3˜3 œ3 3¤3¨3 ¬3°3´3¸3 ¼3À3Ä3È3 Ì3Ğ3Ô3Ø3 Ü3à3ä3è3 ì3ğ3ô3ø3 ü3 444 4444 4 4$4(4 ,4044484 <4@4D4H4 L4P4T4X4 \4`4d4h4 l4p4t4x4 |4€4„4ˆ4 Œ44”4˜4 œ4 4¤4¨4 ¬4°4´4¸4 ¼4À4Ä4È4 Ì4Ğ4Ô4Ø4 Ü4à4ä4è4 ì4ğ4ô4ø4 ü4 555 5555 5 5$5(5 ,5054585 <5@5D5H5 L5P5T5X5 \5`5d5h5 l5p5t5x5 |5€5„5ˆ5 Œ55”5˜5 œ5 5¤5¨5 ¬5°5´5¸5 ¼5À5Ä5È5 Ì5Ğ5Ô5Ø5 Ü5à5ä5è5 ì5ğ5ô5ø5 ü5 666 6666 6 6$6(6 ,6064686 <6@6D6H6 L6P6T6X6 \6`6d6h6 l6p6t6x6 |6€6„6ˆ6 Œ66”6˜6 œ6 6¤6¨6 ¬6°6´6¸6 ¼6À6Ä6È6 Ì6Ğ6Ô6Ø6 Ü6à6ä6è6 ì6ğ6ô6ø6 ü6 777 7777 7 7$7(7 ,7074787 <7@7D7H7 L7P7T7X7 \7`7d7h7 l7p7t7x7 |7€7„7ˆ7 Œ77”7˜7 œ7 7¤7¨7 ¬7°7´7¸7 ¼7À7Ä7È7 Ì7Ğ7