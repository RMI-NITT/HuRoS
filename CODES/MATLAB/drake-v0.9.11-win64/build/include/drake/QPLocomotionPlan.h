#ifndef SYSTEMS_ROBOTINTERFACES_QPLOCOMOTIONPLAN_H_
#define SYSTEMS_ROBOTINTERFACES_QPLOCOMOTIONPLAN_H_

#include <vector>
#include <map>
#include <string>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "PiecewisePolynomial.h"
#include "ExponentialPlusPiecewisePolynomial.h"
#include "RigidBodyManipulator.h"
#include "lcmtypes/drake/lcmt_qp_controller_input.hpp"
#include "BodyMotionData.h"
#include "Side.h"
#include <lcm/lcm-cpp.hpp>
#include "zmpUtil.h"
 

class QuadraticLyapunovFunction {
  // TODO: move into its own file
  // TODO: make part of a Lyapunov function class hierarchy
  // TODO: more functionality
private:
  Eigen::MatrixXd S;
  ExponentialPlusPiecewisePolynomial<double> s1;

public:
  QuadraticLyapunovFunction() { }

  template<typename DerivedS>
  QuadraticLyapunovFunction(const Eigen::MatrixBase<DerivedS>& S, const ExponentialPlusPiecewisePolynomial<double>& s1) :
      S(S), s1(s1) { }

  const Eigen::MatrixXd& getS() const
  {
    return S;
  }

  const ExponentialPlusPiecewisePolynomial<double>& getS1() const
  {
    return s1;
  }
  void setS1(ExponentialPlusPiecewisePolynomial<double>& new_s1)
  {
    s1 = new_s1;
  }

};

struct RigidBodySupportStateElement {
  // TODO: turn this into a class with more functionality
  // TODO: consolidate with SupportStateElement?
  int body; // TODO: should probably be a RigidBody smart pointer
  Eigen::Matrix3Xd contact_points;
  bool use_contact_surface;
  Eigen::Vector4d support_surface; // TODO: should probably be a different type
};

typedef std::vector<RigidBodySupportStateElement> RigidBodySupportState;


enum SupportLogicType {
  REQUIRE_SUPPORT, ONLY_IF_FORCE_SENSED, ONLY_IF_KINEMATIC, KINEMATIC_OR_SENSED, PREVENT_SUPPORT
};

struct KneeSettings {
  double min_knee_angle;
  double knee_kp;
  double knee_kd;
  double knee_weight;
};

struct QPLocomotionPlanSettings {

  QPLocomotionPlanSettings() : min_foot_shift_delay(0.1) {};

  double duration;
  std::vector<RigidBodySupportState> supports;
  std::vector<double> support_times; // length: supports.size() + 1
  typedef std::map<std::string, Eigen::Matrix3Xd> ContactNameToContactPointsMap;
  std::vector<ContactNameToContactPointsMap> contact_groups; // one for each RigidBody
  std::vector<bool> planned_support_command;
  double early_contact_allowed_fraction;

  std::vector<BodyMotionData> body_motions;
  PiecewisePolynomial<double> zmp_trajectory;
  TVLQRData zmp_data;
  Eigen::MatrixXd D_control;
  QuadraticLyapunovFunction V;
  PiecewisePolynomial<double> q_traj;
  ExponentialPlusPiecewisePolynomial<double> com_traj;

  std::string gain_set;
  double mu;
  bool use_plan_shift;
  std::vector<Eigen::DenseIndex> plan_shift_body_motion_indices;
  double g;
  double min_foot_shift_delay; // seconds to wait before updating foot-specific plan shifts (default value set in the constructor above)
  bool is_quasistatic;
  KneeSettings knee_settings;
  double ankle_limits_tolerance;
  std::string pelvis_name;
  std::map<Side, std::string> foot_names;
  std::map<Side, std::string> knee_names;
  std::map<Side, std::string> aky_names;
  std::map<Side, std::string> akx_names;
  double zmp_safety_margin;
  std::vector<int> constrained_position_indices;
  std::vector<int> untracked_position_indices;

  void addSupport(const RigidBodySupportState& support_state, const ContactNameToContactPointsMap& contact_group_name_to_contact_points, double duration) {
    supports.push_back(support_state);
    contact_groups.push_back(contact_group_name_to_contact_points);
    if (support_times.empty())
      support_times.push_back(0.0);
    support_times.push_back(support_times[support_times.size() - 1] + duration);
  }

  // may be useful later
  static KneeSettings createDefaultKneeSettings() {
    KneeSettings knee_settings;
    knee_settings.min_knee_angle = 0.7;
    knee_settings.knee_kp = 40.0;
    knee_settings.knee_kd = 4.0;
    knee_settings.knee_weight = 1.0;
    return knee_settings;
  }

  // may be useful later in setting up constrained_position_indices
  static std::vector<int> findPositionIndices(RigidBodyMan         <v3>46476</v3>
        </triangle>
        <triangle>
          <v1>46490</v1>
          <v2>46489</v2>
          <v3>46475</v3>
        </triangle>
        <triangle>
          <v1>46491</v1>
          <v2>46476</v2>
          <v3>46477</v3>
        </triangle>
        <triangle>
          <v1>46491</v1>
          <v2>46490</v2>
          <v3>46476</v3>
        </triangle>
        <triangle>
          <v1>46492</v1>
          <v2>46477</v2>
          <v3>46478</v3>
        </triangle>
        <triangle>
          <v1>46492</v1>
          <v2>46491</v2>
          <v3>46477</v3>
        </triangle>
        <triangle>
          <v1>46493</v1>
          <v2>46478</v2>
          <v3>46479</v3>
        </triangle>
        <triangle>
          <v1>46493</v1>
          <v2>46492</v2>
          <v3>46478</v3>
        </triangle>
        <triangle>
          <v1>46494</v1>
          <v2>46479</v2>
          <v3>46480</v3>
        </triangle>
        <triangle>
          <v1>46494</v1>
          <v2>46493</v2>
          <v3>46479</v3>
        </triangle>
        <triangle>
          <v1>46495</v1>
          <v2>46480</v2>
          <v3>46481</v3>
        </triangle>
        <triangle>
          <v1>46495</v1>
          <v2>46494</v2>
          <v3>46480</v3>
        </triangle>
        <triangle>
          <v1>46496</v1>
          <v2>46481</v2>
          <v3>46482</v3>
        </triangle>
        <triangle>
          <v1>46496</v1>
          <v2>46495</v2>
          <v3>46481</v3>
        </triangle>
        <triangle>
          <v1>46497</v1>
          <v2>46482</v2>
          <v3>46483</v3>
        </triangle>
        <triangle>
          <v1>46497</v1>
          <v2>46496</v2>
          <v3>46482</v3>
        </triangle>
        <triangle>
          <v1>46498</v1>
          <v2>46483</v2>
          <v3>46486</v3>
        </triangle>
        <triangle>
          <v1>46498</v1>
          <v2>46497</v2>
          <v3>46483</v3>
        </triangle>
        <triangle>
          <v1>46488</v1>
          <v2>46499</v2>
          <v3>46486</v3>
        </triangle>
        <triangle>
          <v1>46501</v1>
          <v2>46486</v2>
          <v3>46499</v3>
        </triangle>
        <triangle>
          <v1>46501</v1>
          <v2>46498</v2>
          <v3>46486</v3>
        </triangle>
        <triangle>
          <v1>46502</v1>
          <v2>46499</v2>
          <v3>46488</v3>
        </triangle>
        <triangle>
          <v1>46503</v1>
          <v2>46501</v2>
          <v3>46499</v3>
        </triangle>
        <triangle>
          <v1>46502</v1>
          <v2>46503</v2>
          <v3>46499</v3>
        </triangle>
        <triangle>
          <v1>46502</v1>
          <v2>46488</v2>
          <v3>46487</v3>
        </triangle>
        <triangle>
          <v1>45845</v1>
          <v2>45841</v2>
          <v3>46489</v3>
        </triangle>
        <triangle>
          <v1>46504</v1>
          <v2>46489</v2>
          <v3>46490</v3>
        </triangle>
        <triangle>
          <v1>46504</v1>
          <v2>45845</v2>
          <v3>46489</v3>
        </triangle>
        <triangle>
          <v1>46505</v1>
          <v2>46490</v2>
          <v3>46491</v3>
        </triangle>
        <triangle>
          <v1>46505</v1>
          <v2>46504</v2>
          <v3>46490</v3>
        </triangle>
        <triangle>
          <v1>46506</v1>
          <v2>46491</v2>
          <v3>46492</v3>
        </triangle>
        <triangle>
          <v1>46506</v1>
          <v2>46505</v2>
          <v3>46491</v3>
        </triangle>
        <triangle>
          <v1>46507</v1>
          <v2>46492</v2>
          <v3>46493</v3>
        </triangle>
        <triangle>
          <v1>46507</v1>
          <v2>46506</v2>
          <v3>46492</v3>
        </triangle>
        <triangle>
          <v1>46508</v1>
          <v2>46493</v2>
          <v3>46494</v3>
        </triangle>
        <triangle>
          <v1>46508</v1>
          <v2>46507</v2>
          <v3>46493</v3>
        </triangle>
        <triangle>
          <v1>46509</v1>
          <v2>46494</v2>
          <v3>46495</v3>
        </triangle>
        <triangle>
          <v1>46509</v1>
          <v2>46508</v2>
          <v3>46494</v3>
        </triangle>
     