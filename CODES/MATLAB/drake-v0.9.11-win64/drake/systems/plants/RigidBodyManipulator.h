#ifndef __RigidBodyManipulator_H__
#define __RigidBodyManipulator_H__

#include <Eigen/Dense>
#include <Eigen/LU>
#include <set>
#include <map>
#include <Eigen/StdVector>

#include "collision/DrakeCollision.h"
#include "shapes/DrakeShapes.h"
#include "KinematicPath.h"
#include "ForceTorqueMeasurement.h"
#include "GradientVar.h"
#include <stdexcept>


#undef DLLEXPORT_RBM
#if defined(WIN32) || defined(WIN64)
  #if defined(drakeRBM_EXPORTS)
    #define DLLEXPORT_RBM __declspec( dllexport )
  #else
    #define DLLEXPORT_RBM __declspec( dllimport )
  #endif
#else
  #define DLLEXPORT_RBM
#endif

#include "RigidBody.h"
#include "RigidBodyFrame.h"
#include "KinematicsCache.h"

#define BASIS_VECTOR_HALF_COUNT 2  //number of basis vectors over 2 (i.e. 4 basis vectors in this case)
#define EPSILON 10e-8
#define MIN_RADIUS 1e-7

typedef Eigen::Matrix<double, 3, BASIS_VECTOR_HALF_COUNT> Matrix3kd;

class DLLEXPORT_RBM RigidBodyActuator
{
public:
  RigidBodyActuator(std::string _name, std::shared_ptr<RigidBody> _body, double _reduction = 1.0) :
    name(_name), body(_body), reduction(_reduction) {};

  std::string name;
  std::shared_ptr<RigidBody> body;
  double reduction;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class DLLEXPORT_RBM RigidBodyLoop
{
public:
  RigidBodyLoop(const std::shared_ptr<RigidBodyFrame>& _frameA, const std::shared_ptr<RigidBodyFrame>& _frameB, const Eigen::Vector3d& _axis) :
    frameA(_frameA), frameB(_frameB), axis(_axis) {};

  std::shared_ptr<RigidBodyFrame> frameA, frameB;
  Eigen::Vector3d axis;

  friend std::ostream& operator<<(std::ostream& os, const RigidBodyLoop& obj);

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class DLLEXPORT_RBM RigidBodyManipulator
{
public:
  RigidBodyManipulator(const std::string &urdf_filename, const DrakeJoint::FloatingBaseType floating_base_type = DrakeJoint::ROLLPITCHYAW);
  RigidBodyManipulator(void);
  virtual ~RigidBodyManipulator(void);

  bool addRobotFromURDFString(const std::string &xml_string, const std::string &root_dir = ".", const DrakeJoint::FloatingBaseType floating_base_type = DrakeJoint::ROLLPITCHYAW);
  bool addRobotFromURDFString(const std::string &xml_string, std::map<std::string,std::string>& package_map, const std::string &root_dir = ".", const DrakeJoint::FloatingBaseType floating_base_type = DrakeJoint::ROLLPITCHYAW);
  bool addRobotFromURDF(const std::string &urdf_filename, const DrakeJoint::FloatingBaseType floating_base_type = DrakeJoint::ROLLPITCHYAW);
  bool addRobotFromURDF(const std::string &urdf_filename, std::map<std::string,std::string>& package_map, const DrakeJoint::FloatingBaseType floating_base_type = DrakeJoint::ROLLPITCHYAW);

  void addFrame(const std::shared_ptr<RigidBodyFrame>& frame);

  std::map<std::string, int> computePositionNameToIndexMap() const;

  void surfaceTangents(Eigen::Map<Eigen::Matrix3Xd> const & normals, std::vector< Eigen::Map<Eigen::Matrix3Xd> > & tangents) const;

  void compile(void);  // call me after the model is loaded

  void getRandomConfiguration(Eigen::VectorXd& q, std::default_random_engine& generator) const;

  // akin to the coordinateframe names in matlab
  std::string getPositionName(int position_num) const;
  std::string getVelocityName(int velocity_num) const;
  std::string getStateName(int state_num) const;

  template <typename DerivedQ>
  KinematicsCache<typename DerivedQ::Scalar> doKinematics(const Eigen::MatrixBase<DerivedQ>& q, int gradient_order) {
    KinematicsCache<typename DerivedQ::Scalar> ret(bodies, gradient_order);
    ret.initialize(q);
    doKinematics(ret);
    return ret;
  }

  template <typename DerivedQ, typename DerivedV>
  KinematicsCache<typename DerivedQ::Scalar> doKinematics(const Eigen::MatrixBase<DerivedQ>& q, const Eigen::MatrixBase<DerivedV>& v, int gradient_order = 0, bool compute_JdotV = true) {
    KinematicsCache<typename DerivedQ::Scalar> ret(bodies, gradient_order);
    ret.initialize(q, v);
    doKinematics(ret, compute_JdotV);
    return ret;
  }

  template <typename Scalar>
  void doKinematics(KinematicsCache<Scalar>& cache, bool compute_JdotV = false) const {
    using namespace std;
    using namespace Eigen;

    const auto& q = cache.getQ();
    if (!initialized)
      throw runtime_error("RigidBodyManipulator::doKinematics: call compile first.");

    int nq = num_positions;
    int gradient_order = cache.getGradientOrder();
    bool compute_gradients = gradient_order > 0;

    compute_JdotV = compute_JdotV && cache.hasV(); // no sense in computing Jdot times v if v is not passed in

    cache.setPositionKinematicsCached(); // doing this here because there is a geometricJacobian call within doKinematics below which checks for this

    for (int i = 0; i < bodies.size(); i++) {
      RigidBody& body = *bodies[i];
      KinematicsCacheElement<Scalar>& element = cache.getElement(body);

      if (body.hasParent()) {
        const KinematicsCacheElement<Scalar>& parent_element = cache.getElement(*body.parent);
        const DrakeJoint& joint = body.getJoint();
        auto q_body = q.middleRows(body.position_num_start, joint.getNumPositions());

        // transform
        auto T_body_to_parent = joint.getTransformToParentBody().cast<Scalar>() * joint.jointTransform(q_body);
        element.transform_to_world = parent_element.transform_to_world * T_body_to_parent;

        // motion subspace in body frame
        Matrix<Scalar, Dynamic, Dynamic>* dSdq = compute_gradients ? &(element.motion_subspace_in_body.gradient().value()) : nullptr;
        joint.motionSubspace(q_body, element.motion_subspace_in_body.value(), dSdq);

        // motion subspace in world frame
        element.motion_subspace_in_world.value() = transformSpatialMotion(element.transform_to_world, element.motion_subspace_in_body.value());

        // qdot to v, v to qdot
        if (compute_gradients) {
          // TODO: make DrakeJoint::v2qdot and qdot2v accept Refs instead, pass in blocks of element.v_to_qdot.gradient().value() and element.qdot_to_v.gradient().value()
          Matrix<Scalar, Dynamic, Dynamic> dqdot_to_vdqi(element.qdot_to_v.value().size(), joint.getNumPositions());
          joint.qdot2v(q_body, element.qdot_to_v.value(), &dqdot_to_vdqi);
          auto& dqdot_to_v = element.qdot_to_v.gradient().value();
          dqdot_to_v.setZero();
          dqdot_to_v.middleCols(body.position_num_start, joint.getNumPositions()) = dqdot_to_vdqi;

          Matrix<Scalar, Dynamic, Dynamic> dv_to_qdotdqi(element.v_to_qdot.value().size(), joint.getNumPositions());
          joint.v2qdot(q_body, element.v_to_qdot.value(), &dv_to_qdotdqi);
          auto& dv_to_qdot = element.v_to_qdot.gradient().value();
          dv_to_qdot.setZero();
          dv_to_qdot.middleCols(body.position_num_start, joint.getNumPositions()) = dv_to_qdotdqi;
        }
        else {
          joint.qdot2v(q_body, element.qdot_to_v.value(), nullptr);
          joint.v2qdot(q_body, element.v_to_qdot.value(), nullptr);
        }

        if (compute_gradients) {
          // gradient of transform
          auto dT_body_to_parentdqi = dHomogTrans(T_body_to_parent, element.motion_subspace_in_body.value(), element.qdot_to_v.value()).eval();
          typename Gradient<typename Transform<Scalar, 3, Isometry>::MatrixType, Eigen::Dynamic>::type dT_body_to_parentdq(HOMOGENEOUS_TRANSFORM_SIZE, nq);
          dT_body_to_parentdq.setZero();
          dT_body_to_parentdq.middleCols(body.position_num_start, joint.getNumPositions()) = dT_body_to_parentdqi;
          element.dtransform_to_world_dq = matGradMultMat(parent_element.transform_to_world.matrix(), T_body_to_parent.matrix(), parent_element.dtransform_to_world_dq, dT_body_to_parentdq);

          // gradient of motion subspace in world
          Matrix<Scalar, Dynamic, Dynamic> dSdq(element.motion_subspace_in_body.value().size(), nq);
          dSdq.setZero();
          dSdq.middleCols(body.position_num_start, joint.getNumPositions()) = element.motion_subspace_in_body.gradient().value();
          element.motion_subspace_in_world.gradient().value() = dTransformSpatialMotion(element.transform_to_world, element.motion_subspace_in_body.value(), element.dtransform_to_world_dq, dSdq)x>
        <vertex>
          <coordinates>
            <x>12.953136</x>
            <y>15.591578</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.906164</x>
            <y>15.78469</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.866907</x>
            <y>15.979517</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.835427</x>
            <y>16.17575</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.811772</x>
            <y>16.37308</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.795982</x>
            <y>16.571194</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.788081</x>
            <y>16.769779</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.788081</x>
            <y>16.968521</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.795982</x>
            <y>17.167107</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.811772</x>
            <y>17.365221</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.835427</x>
            <y>17.56255</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.866907</x>
            <y>17.758784</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.906164</x>
            <y>17.95361</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.953136</x>
            <y>18.146722</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.007747</x>
            <y>18.337814</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.069913</x>
            <y>18.526583</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.139535</x>
            <y>18.712732</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.216502</x>
            <y>18.895966</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.300693</x>
            <y>19.075995</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.391975</x>
            <y>19.252534</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.490203</x>
            <y>19.425305</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.595223</x>
            <y>19.594034</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.706867</x>
            <y>19.758454</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.82496</x>
            <y>19.918306</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.949315</x>
            <y>20.073336</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.079735</x>
            <y>20.2233</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.216013</x>
            <y>20.36796</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.357935</x>
            <y>20.507088</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.505277</x>
            <y>20.640464</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.657804</x>
            <y>20.767877</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.815276</x>
            <y>20.889124</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.977444</x>
            <y>21.004016</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.144052</x>
            <y>21.112369</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.314836</x>
            <y>21.214013</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.489525</x>
            <y>21.308787</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.667845</x>
            <y>21.39654</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.849512</x>
            <y>21.477135</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.034239</x>
            <y>21.550444</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.221736</x>
            <y>21.616351</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.411704</x>
            <y>21.674752</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.603844</x>
            <y>21.725554</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.797851</x>
            <y>21.768676</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.99342</x>
            <y>21.804052</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.19024</x>
            <y>21.831625</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.388001</x>
            <y>21.851351</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.58639</x>
            <y>21.863199</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.785093</x>
            <y>21.86715</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.983796</x>
            <y>21.863199</y>
            <z>8.4082</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.785093</x>
            <y>11.87115</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.983796</x>
            <y>11.875102</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.58639</x>
            <y>11.875102</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.388001</x>
            <y>11.88695</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.19024</x>
            <y>11.906675</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.99342</x>
            <y>11.934248</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.797851</x>
            <y>11.969624</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.603844</x>
            <y>12.012747</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.411704</x>
            <y>12.063549</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.221736</x>
            <y>12.121949</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.034239</x>
            <y>12.187856</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.849512</x>
            <y>12.261165</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.667845</x>
            <y>12.34176</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.489525</x>
            <y>12.429514</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.314836</x>
            <y>12.524287</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.144052</x>
            <y>12.625931</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.977444</x>
            <y>12.734284</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.815276</x>
            <y>12.849176</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.657804</x>
            <y>12.970424</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.505277</x>
            <y>13.097836</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.357935</x>
            <y>13.231212</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.216013</x>
            <y>13.37034</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.079735</x>
            <y>13.515</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.949315</x>
            <y>13.664964</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.82496</x>
            <y>13.819995</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.706867</x>
            <y>13.979846</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.595223</x>
            <y>14.144267</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.490203</x>
            <y>14.312996</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.391975</x>
            <y>14.485766</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.300693</x>
            <y>14.662306</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.216502</x>
            <y>14.842335</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.139535</x>
            <y>15.025568</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.069913</x>
            <y>15.211717</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.007747</x>
            <y>15.400486</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.953136</x>
            <y>15.591578</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.906164</x>
            <y>15.78469</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.866907</x>
            <y>15.979517</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.835427</x>
            <y>16.17575</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.811772</x>
            <y>16.37308</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.795982</x>
            <y>16.571194</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.788081</x>
            <y>16.769779</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.788081</x>
            <y>16.968521</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.795982</x>
            <y>17.167107</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.811772</x>
            <y>17.365221</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.835427</x>
            <y>17.56255</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.866907</x>
            <y>17.758784</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.906164</x>
            <y>17.95361</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>12.953136</x>
            <y>18.146722</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.007747</x>
            <y>18.337814</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.069913</x>
            <y>18.526583</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.139535</x>
            <y>18.712732</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.216502</x>
            <y>18.895966</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.300693</x>
            <y>19.075995</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.391975</x>
            <y>19.252534</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.490203</x>
            <y>19.425305</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.595223</x>
            <y>19.594034</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.706867</x>
            <y>19.758454</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.82496</x>
            <y>19.918306</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>13.949315</x>
            <y>20.073336</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.079735</x>
            <y>20.2233</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.216013</x>
            <y>20.36796</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.357935</x>
            <y>20.507088</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.505277</x>
            <y>20.640464</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.657804</x>
            <y>20.767877</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.815276</x>
            <y>20.889124</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>14.977444</x>
            <y>21.004016</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.144052</x>
            <y>21.112369</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.314836</x>
            <y>21.214013</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.489525</x>
            <y>21.308787</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.667845</x>
            <y>21.39654</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>15.849512</x>
            <y>21.477135</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.034239</x>
            <y>21.550444</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.221736</x>
            <y>21.616351</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.411704</x>
            <y>21.674752</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.603844</x>
            <y>21.725554</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.797851</x>
            <y>21.768676</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.99342</x>
            <y>21.804052</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.19024</x>
            <y>21.831625</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.388001</x>
            <y>21.851351</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.58639</x>
            <y>21.863199</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.785093</x>
            <y>21.86715</y>
            <z>8.8086</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.983796</x>
            <y>21.863199</y>
            <z>8.6084</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.785093</x>
            <y>11.87115</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.983796</x>
            <y>11.875102</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.58639</x>
            <y>11.875102</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.388001</x>
            <y>11.88695</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>17.19024</x>
            <y>11.906675</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.99342</x>
            <y>11.934248</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.797851</x>
            <y>11.969624</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.603844</x>
            <y>12.012747</y>
            <z>9.0088</z>
          </coordinates>
        </vertex>
        <vertex>
          <coordinates>
            <x>16.4117