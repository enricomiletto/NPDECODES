#ifndef STABLE_EVALUATION_AT_A_POINT_H
#define STABLE_EVALUATION_AT_A_POINT_H

/**
 * @file stableevaluationatapoint.h
 * @brief NPDE homework StableEvaluationAtAPoint
 * @author Amélie Loher & Erick Schulz
 * @date 22/04/2020
 * @copyright Developed at ETH Zurich
 */

#include <lf/base/base.h>
#include <lf/geometry/geometry.h>
#include <lf/mesh/utils/utils.h>
#include <lf/quad/quad.h>
#include <lf/uscalfe/uscalfe.h>
#include<lf/fe/fe.h>

#include <Eigen/Core>
#include <cmath>
#include <complex>
#include <algorithm>

namespace StableEvaluationAtAPoint {

/** @brief Approximates the mesh size for the given mesh.
 *
 * @param mesh_p pointer to a LehreFEM++ mesh object
 */
double MeshSize(const std::shared_ptr<const lf::mesh::Mesh> &mesh_p);

/** @brief Returns the outer normal of the unit squre at point x
 *  @param x point on the boundary of the unit squre 
 */
Eigen::Vector2d OuterNormalUnitSquare(Eigen::Vector2d x);


class FundamentalSolution{
public:
  FundamentalSolution(Eigen::Vector2d x): x_{x} {}

  /** @brief  Returns fundamental solution G_x(y) */
  double operator()(Eigen::Vector2d y);
  /** @brief Returns the gradient of the fundamental solution G_x(y) */
  Eigen::Vector2d grad(Eigen::Vector2d y);

private:
  Eigen::Vector2d x_;
};


// /** @brief Returns fundamental solution G(x,y).
//  *  @param x, y: point coordinate vectors
//  */
// double G(Eigen::Vector2d x, Eigen::Vector2d y) {
//   double res;
//   LF_ASSERT_MSG(x != y, "G not defined for these coordinates!");
//   res = (-1.0 / (2.0 * M_PI)) * std::log((x - y).norm());
//   return res;
// }

// /** @brief Returns the gradient of the fundamental solution G(x,y).
//  *  @param x, y: point coordinate vectors
//  */
// Eigen::Vector2d gradG(Eigen::Vector2d x, Eigen::Vector2d y) {
//   Eigen::Vector2d res;
//   LF_ASSERT_MSG(x != y, "G not defined for these coordinates!");
//   res = (x - y) / (2.0 * M_PI * (x - y).squaredNorm());
//   return res;
// }

/** @brief Evaluates the Integral P_SL using the local midpoint rule
 * on the partitioning of the boundary of Omega induced by the mesh.
 * @warning The supplied mesh object must hold a triangulation of the **unit
 * square**. This functions only works in this particular setting
 */
/* SAM_LISTING_BEGIN_1 */
template <typename FUNCTOR>
double PSL(std::shared_ptr<const lf::mesh::Mesh> mesh, FUNCTOR &&v,
           const Eigen::Vector2d x) {
  double PSLval = 0.0;
  FundamentalSolution G(x);
#if SOLUTION
  //Flag edges on the boundary
  auto bd_flags_edge{lf::mesh::utils::flagEntitiesOnBoundary(mesh, 1)};
  
  // Loop over boundary edges
  for (const lf::mesh::Entity *e : mesh->Entities(1)) {
    if (bd_flags_edge(*e)) {
      const lf::geometry::Geometry *geo_ptr = e->Geometry();
      LF_ASSERT_MSG(geo_ptr != nullptr, "Missing geometry!");
      
      // Fetch coordinates of corner points
      const Eigen::Matrix2d corners = lf::geometry::Corners(*geo_ptr);
      // Determine midpoint of edges
      const Eigen::Vector2d midpoint{0.5 * (corners.col(0) + corners.col(1))};

      // Compute and add the edge contribution
      PSLval += v(midpoint) * G(midpoint) * lf::geometry::Volume(*geo_ptr);
    }
  }
#else
  //====================
  // Your code goes here
  //====================
#endif
  return PSLval;
}
/* SAM_LISTING_END_1 */

/** @brief Evaluates the Integral P_DL using the local midpoint rule
 * on the partitioning of the boundary of Omega induced by the mesh.
 *
 * @warning The supplied mesh object must hold a triangulation of the **unit
 * square**. This functions only works in this particular setting
 *
 */
/* SAM_LISTING_BEGIN_2 */
template <typename FUNCTOR>
double PDL(std::shared_ptr<const lf::mesh::Mesh> mesh, FUNCTOR &&v,
           const Eigen::Vector2d x) {
  double PDLval = 0.0;
  FundamentalSolution G(x);
#if SOLUTION
  //Flag edges on the boundary
  auto bd_flags_edge{lf::mesh::utils::flagEntitiesOnBoundary(mesh, 1)};

  // Loop over boundary edges
  for (const lf::mesh::Entity *e : mesh->Entities(1)) {
    if (bd_flags_edge(*e)) {
      const lf::geometry::Geometry *geo_ptr = e->Geometry();
      LF_ASSERT_MSG(geo_ptr != nullptr, "Missing geometry!");
      
      // Fetch coordinates of corner points
      Eigen::MatrixXd corners = lf::geometry::Corners(*geo_ptr);
      // Determine midpoints of edges
      const Eigen::Vector2d midpoint{0.5 * (corners.col(0) + corners.col(1))};
  
      // Determine the normal vector n on the unit square.
      Eigen::Vector2d n = OuterNormalUnitSquare(midpoint);
    
      // Compute and the elemental contribution
      PDLval += v(midpoint) * (G.grad(midpoint)).dot(n) *
                lf::geometry::Volume(*geo_ptr);
    }
  }
#else
  //====================
  // Your code goes here
  //====================
#endif
  return PDLval;
}
/* SAM_LISTING_END_2 */

/* SAM_LISTING_BEGIN_3 */
/** @brief  This function computes u(x) = P_SL(grad u * n) - P_DL(u).
 *
 * For u(x) = log( (x + (1, 0)^T).norm() ) and x = (0.3, 0.4)^T,
 * it computes the difference between the analytical and numerical
 * evaluation of u.
 *
 * @warning The supplied mesh object must hold a triangulation of the **unit
 * square**. This functions only works in this particular setting.
 */
double PointEval(std::shared_ptr<const lf::mesh::Mesh> mesh);
/* SAM_LISTING_END_3 */


class Psi{
public:
  Psi(Eigen::Vector2d center): center_(center) {}
  
  //computes Psi_x(y)
  double operator()(Eigen::Vector2d y);
  //computes grad(Psi_x)(y)
  Eigen::Vector2d grad(Eigen::Vector2d y);
  //computes the laplacian of Psi_x at y
  double lapl(Eigen::Vector2d y);

private:
  Eigen::Vector2d center_ = Eigen::Vector2d(0.5,0.5);
};

// /* Computes Psi_x(y). */
// double Psi(const Eigen::Vector2d y); {
//   double Psi_xy;
//   const Eigen::Vector2d half(0.5, 0.5);
//   const double constant = M_PI / (0.5 * std::sqrt(2) - 1.0);
//   const double dist = (y - half).norm();

//   if (dist <= 0.25 * std::sqrt(2)) {
//     Psi_xy = 0.0;
//   } else if (dist >= 0.5) {
//     Psi_xy = 1.0;
//   } else {
//     Psi_xy = std::pow(std::cos(constant * (dist - 0.5)), 2);
//   }
//   return Psi_xy;
// }

// /* Computes grad(Psi_x(y)). */
// Eigen::Vector2d gradPsi(const Eigen::Vector2d y) {
//   Eigen::Vector2d gradPsi_xy;
//   Eigen::Vector2d half(0.5, 0.5);
//   double constant = M_PI / (0.5 * std::sqrt(2) - 1.0);
//   double dist = (y - half).norm();

//   if (dist <= 0.25 * std::sqrt(2)) {
//     gradPsi_xy(0) = 0.0;
//     gradPsi_xy(1) = 0.0;

//   } else if (dist >= 0.5) {
//     gradPsi_xy(0) = 0.0;
//     gradPsi_xy(1) = 0.0;

//   } else {
//     gradPsi_xy = -2.0 * std::cos(constant * (dist - 0.5)) *
//                  std::sin(constant * (dist - 0.5)) * (constant / dist) *
//                  (y - half);
//   }
//   return gradPsi_xy;
// }

/* Computes Laplacian of Psi_x(y). */
// double laplPsi(const Eigen::Vector2d y) {
//   double laplPsi_xy;
//   Eigen::Vector2d half(0.5, 0.5);
//   double constant = M_PI / (0.5 * std::sqrt(2) - 1.0);
//   double dist = (y - half).norm();

//   if (dist <= 0.25 * std::sqrt(2)) {
//     laplPsi_xy = 0.0;
//   } else if (dist >= 0.5) {
//     laplPsi_xy = 0.0;
//   } else {
//     laplPsi_xy =
//         (2 * std::pow(constant, 2) / (y - half).squaredNorm()) *
//             (y - half).dot(y - half) *
//             (std::pow(std::sin(constant * (dist - 0.5)), 2) -
//              std::pow(std::cos(constant * (dist - 0.5)), 2)) -
//         (2 * constant / dist) * std::cos(constant * (dist - 0.5)) *
//             std::sin(constant * (dist - 0.5)) *
//             (1.0 - ((y - half).dot(y - half) / (y - half).squaredNorm()));
//   }
//   return laplPsi_xy;
// }

/* Computes Jstar
 * fe_space: finite element space defined on a triangular mesh of the square
 * domain u: Function handle for u x: Coordinate vector for x
 */
/* SAM_LISTING_BEGIN_4 */
double Jstar(std::shared_ptr<lf::uscalfe::FeSpaceLagrangeO1<double>> fe_space,
             Eigen::VectorXd uFE, const Eigen::Vector2d x); 

/* SAM_LISTING_END_4 */

/* Evaluates u(x) according to (3.11.14).
 * u: Function Handle for u
 * x: Coordinate vector for x
 */
double StablePointEvaluation(
    std::shared_ptr<lf::uscalfe::FeSpaceLagrangeO1<double>> fe_space,
    Eigen::VectorXd uFE, const Eigen::Vector2d x); 

template <typename FUNCTOR>
Eigen::VectorXd SolveBVP(
    const std::shared_ptr<lf::uscalfe::FeSpaceLagrangeO1<double>> &fe_space_p,
    FUNCTOR &&u) {
  Eigen::VectorXd discrete_solution;

  // TOOLS AND DATA
  // Pointer to current mesh
  std::shared_ptr<const lf::mesh::Mesh> mesh_p = fe_space_p->Mesh();
  // Obtain local->global index mapping for current finite element space
  const lf::assemble::DofHandler &dofh{fe_space_p->LocGlobMap()};
  // Dimension of finite element space
  const lf::uscalfe::size_type N_dofs(dofh.NumDofs());
  // Obtain specification for shape functions on edges
  std::shared_ptr<const lf::fe::ScalarReferenceFiniteElement<double>>
      rsf_edge_p (fe_space_p->ShapeFunctionLayout(lf::base::RefEl::kSegment()) );

  // Dirichlet data
  auto mf_g = lf::mesh::utils::MeshFunctionGlobal(
      [u](Eigen::Vector2d x) -> double { return u(x); });
  // Right-hand side source function f
  auto mf_f = lf::mesh::utils::MeshFunctionGlobal(
      [](Eigen::Vector2d x) -> double { return 0.0; });

  // I : ASSEMBLY
  // Matrix in triplet format holding Galerkin matrix, zero initially.
  lf::assemble::COOMatrix<double> A(N_dofs, N_dofs);
  // Right hand side vector, must be initialized with 0!
  Eigen::Matrix<double, Eigen::Dynamic, 1> phi(N_dofs);
  phi.setZero();

  // I.i : Computing volume matrix for negative Laplace operator
  // Initialize object taking care of local mass (volume) computations.
  lf::uscalfe::LinearFELaplaceElementMatrix elmat_builder{};
  // Invoke assembly on cells (co-dimension = 0 as first argument)
  // Information about the mesh and the local-to-global map is passed through
  // a Dofhandler object, argument 'dofh'. This function call adds triplets to
  // the internal COO-format representation of the sparse matrix A.
  lf::assemble::AssembleMatrixLocally(0, dofh, dofh, elmat_builder, A);

  // I.ii : Computing right-hand side vector
  lf::uscalfe::ScalarLoadElementVectorProvider<double, decltype(mf_f)>
      elvec_builder(fe_space_p, mf_f);
  // Invoke assembly on cells (codim == 0)
  AssembleVectorLocally(0, dofh, elvec_builder, phi);

  // I.iii : Imposing essential boundary conditions
  // Obtain an array of boolean flags for the edges of the mesh, 'true'
  // indicates that the edge lies on the boundary (codim = 1)
  auto bd_flags{lf::mesh::utils::flagEntitiesOnBoundary(mesh_p, 1)};
  // Inspired by the example in the documentation of
  // InitEssentialConditionFromFunction()
  // https://craffael.github.io/lehrfempp/namespacelf_1_1uscalfe.html#a5afbd94919f0382cf3fb200c452797ac
  // Determine the fixed dofs on the boundary and their values
  // Alternative: See lecturedemoDirichlet() in
  // https://github.com/craffael/lehrfempp/blob/master/examples/lecturedemos/lecturedemoassemble.cc
  auto edges_flag_values_Dirichlet{
      lf::fe::InitEssentialConditionFromFunction(*fe_space_p, 
                                                      bd_flags, mf_g)};
  // Eliminate Dirichlet dofs from the linear system
  lf::assemble::FixFlaggedSolutionCompAlt<double>(
      [&edges_flag_values_Dirichlet](lf::assemble::glb_idx_t gdof_idx) {
        return edges_flag_values_Dirichlet[gdof_idx];
      },
      A, phi);

  // Assembly completed! Convert COO matrix A into CRS format using Eigen's
  // internal conversion routines.
  Eigen::SparseMatrix<double> A_sparse = A.makeSparse();

  // II : SOLVING  THE LINEAR SYSTEM
  // II.i : Setting up Eigen's sparse direct elimination
  Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
  solver.compute(A_sparse);
  LF_VERIFY_MSG(solver.info() == Eigen::Success, "LU decomposition failed");
  // II.ii : Solving
  discrete_solution = solver.solve(phi);
  LF_VERIFY_MSG(solver.info() == Eigen::Success, "Solving LSE failed");

  return discrete_solution;
}; // solveBVP

} /* namespace StableEvaluationAtAPoint */


#endif //STABLE_EVALUATION_AT_A_POINT_H