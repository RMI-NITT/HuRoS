classdef Manipulator < DrakeSystem
% An abstract class that wraps H(q)vdot + C(q,v,f_ext) = B(q)u.

  methods
    function obj = Manipulator(num_q, num_u, num_v)
      if nargin<3, num_v = num_q; end

      obj = obj@DrakeSystem(num_q+num_v,0,num_u,num_q+num_v,false,true);
      obj.num_positions = num_q;
      obj.num_velocities = num_v;
      obj.joint_limit_min = -inf(num_q,1);
      obj.joint_limit_max = inf(num_q,1);
    end
  end

  methods (Abstract=true)
    %  H(q)vdot + C(q,v,f_ext) = Bu
    [H,C,B] = manipulatorDynamics(obj,q,v);
  end

  methods
    function [H,C_times_v,G,B] = manipulatorEquations(obj,q,v)
      % extract the alternative form of the manipulator equations:
      %   H(q)vdot + C(q,v)v + G(q) = B(q)u
      
      if nargin<2,
        checkDependency('spotless');
        q = TrigPoly('q','s','c',getNumPositions(obj));
      end
      if nargin<3,
        v = msspoly('v',getNumVelocities(obj));
      end
      
      [H,C,B] = manipulatorDynamics(obj,q,v);
      [~,G] = manipulatorDynamics(obj,q,0*v);
      C_times_v = C-G;
    end
    
    function [xdot,dxdot] = dynamics(obj,t,x,u)
    % Provides the DrakeSystem interface to the manipulatorDynamics.

      q = x(1:obj.num_positions);
      v = x(obj.num_positions+1:end);

      if (nargout>1)
        if ~isempty(obj.position_constraints) || ~isempty(obj.velocity_constraints)
          % by naming this 'MATLAB:TooManyOutputs', geval will catch the
          % error and use TaylorVarInstead
          error('MATLAB:TooManyOutputs','User gradients for constrained dynamics not implemented yet.');
        end

        % Note: the next line assumes that user gradients are implemented.
        % If it fails, then it will raise the same exception that I would
        % want to raise for this method, stating that not all outputs were
        % assigned.  (since I can't write dxdot anymore)
        [H,C,B,dH,dC,dB] = obj.manipulatorDynamics(q,v);
        Hinv = inv(H);

        if (obj.num_u>0)
          vdot = Hinv*(B*u-C);
          dtau = matGradMult(dB,u) - dC;
          dvdot = [zeros(obj.num_positions,1),...
            -Hinv*matGradMult(dH(:,1:obj.num_positions),vdot) + Hinv*dtau(:,1:obj.num_positions),...
            +Hinv*dtau(:,1+obj.num_positions:end), Hinv*B];
        else
          vdot = -Hinv*C;
          dvdot = [zeros(obj.num_velocities,1),...
            Hinv*(-matGradMult(dH(:,1:obj.num_positions),vdot) - dC(:,1:obj.num_positions)),...
            Hinv*(-dC(:,obj.num_positions+1:end))];
        end

        [VqInv,dVqInv] = vToqdot(obj,q);
        xdot = [VqInv*v;vdot];
        dxdot = [...
          zeros(obj.num_positions,1), matGradMult(dVqInv, v), VqInv, zeros(obj.num_positions,obj.num_u);
          dvdot];
      else
        [H,C,B] = manipulatorDynamics(obj,q,v);
        Hinv = inv(H);
        if (obj.num_u>0) tau=B*u - C; else tau=-C; end
        tau = tau + computeConstraintForce(obj,q,v,H,tau,Hinv);

        vdot = Hinv*tau;
        % note that I used to do this (instead of calling inv(H)):
        %   vdot = H\tau
        % but I already have and use Hinv, so use it again here

        xdot = [vToqdot(obj,q)*v; vdot];
      end

    end

    function [Vq,dVq] = qdotToV(obj, q)
      % defines the linear map v = Vq * qdot
      % default relationship is that v = qdot
      assert(obj.num_positions==obj.num_velocities);
      Vq = eye(length(q));
      dVq = zeros(numel(Vq), obj.num_positions);
    end

    function [VqInv,dVqInv] = vToqdot(obj, q)
      % defines the linear map qdot = Vqinv * v
      % default relationship is that v = qdot
      assert(obj.num_positions==obj.num_velocities);
      VqInv = eye(length(q));
      dVqInv = zeros(numel(VqInv), obj.num_positions);
    end

    function y = output(obj,t,x,u)
      % default output is the full state
      y = x;
    end

  end

  methods (Access=private)
    function constraint_force = computeConstraintForce(obj,q,v,H,tau,Hinv)
      % Helper function to compute the internal forces required to enforce
      % equality constraints

      alpha = 5;  % 1/time constant of position constraint satisfaction (see my latex rigid body notes)
      beta = 0;    % 1/time constant of velocity constraint satisfaction

      phi=[]; psi=[];
      qd = vToqdot(obj, q) * v;
      if ~isempty(obj.position_constraints) && ~isempty(obj.velocity_constraints)
        [phi,J,dJ] = obj.positionConstraints(q);
        Jdotqd = dJ*reshape(qd*qd',obj.num_positions^2,1);

        [psi,dpsi] = obj.velocityConstraints(q,qd);
        dpsidq = dpsi(:,1:obj.num_positions);
        dpsidqd = dpsi(:,obj.num_positions+1:end);

        term1=Hinv*[J;dpsidqd]';
        term2=Hinv*tau;

        constraint_force = -[J;dpsidqd]'*pinv([J*term1;dpsidqd*term1])*[J*term2 + Jdotqd + 2*alpha*J*qd + alpha^2*phi; dpsidqd*term2 + dpsidq*qd + beta*psi];
      elseif ~isempty(obj.position_constraints)  % note: it didn't work to just have dpsidq,etc=[], so it seems like the best solution is to handle each case...
        [phi,J,dJ] = obj.positionConstraints(q);
        % todo: find a way to use Jdot*qd directly (ala Twan's code)
        % instead of computing dJ
        Jdotqd = dJ*reshape(qd*qd',obj.num_positions^2,1);

        constraint_force = -J'*pinv(J*Hinv*J')*(J*Hinv*tau + Jdotqd + 2*alpha*J*qd + alpha^2*phi);
        % useful for debugging:
        % phi, phidot = J*qd, phiddot = J*Hinv*(tau+constraint_force)+Jdotqd
      elseif ~isempty(obj.velocity_constraints)
        [psi,J] = obj.velocityConstraints(q,qd);
        dpsidq = J(:,1:obj.num_positions);
        dpsidqd = J(:,obj.num_positions+1:end);

        constraint_force = -dpsidqd'*pinv(dpsidqd*Hinv*dpsidqd')*(dpsidq*qd + dpsidqd*Hinv*tau+beta*psi);
      else
        constraint_force = 0*q;
      end
    end
  end

  methods
    function [obj,id] = addPositionEqualityConstraint(obj,con,position_ind)
      % Adds a position constraint of the form phi(q) = constant
      % which can be enforced directly in the manipulator dynamics.
      % This method will also register phi (and it's time derivative)
      % as state constraints for the dynamical system.
      if( isa(con, 'DrakeFunctionConstraint') && ~isa(con.fcn, 'drakeFunction.kinematic.RelativePosition'))
        obj.only_loops = false;
      end
      if (nargin<3 || isempty(position_ind)) 
        position_ind = 1:getNumPositions(obj);
      end
      id = numel(obj.position_constraints)+1;
      obj = updatePositionEqualityConstraint(obj,id,con,position_ind);
    end
    
    function obj = updatePositionEqualityConstraint(obj,id,con,position_ind)
      typecheck(con,'Constraint'); % for now
      assert(all(con.lb == con.ub));
      if (nargin<4 || isempty(position_ind)) 
        position_ind = 1:getNumPositions(obj);
      end
        
      if id>numel(obj.position_constraints) % then it's a new constraint
        [obj,obj.position_constraint_ids(1,id)] = addStateConstraint(obj,con,position_ind);
      else 
        obj.num_position_constraints = obj.num_position_constraints-obj.position_constraints{id}.num_cnstr;
        obj = updateStateConstraint(obj,obj.position_constraint_ids(1,id),con,position_ind);
      end
      
      obj.num_position_constraints = obj.num_position_constraints+con.num_cnstr;
      obj.position_constraints{id} = con;
      
%      obj.warning_manager.warnOnce('Drake:Manipulator:Todo','still need to add time derivatives of position constraints');
    end
    
    function [obj,id] = addVelocityEqualityConstraint(obj,con,velocity_ind)
      % Adds a velocity constraint of the form psi(q,v) = constant
      % (with dpsidv~=0) which can be enforced directly in the manipulator dynamics.
      % This method will also register psi as a state constraint
      % for the dynamical system.
      if (nargin<3 || isempty(velocity_ind))
        velocity_ind = 1:getNumVelocities(obj);
      end
      
      id = numel(obj.position_constraints)+1;
      obj = updatePositionEqualityConstraint(obj,id,con);
      
    end
    function obj = updateVelocityEqualityConstraint(obj,id,con,velocity_ind)
      typecheck(con,'Constraint'); % for now
      assert(all(con.lb == con.ub));
      if (nargin<4 || ise3>
        </triangle>
        <triangle>
          <v1>22903</v1>
          <v2>22885</v2>
          <v3>22886</v3>
        </triangle>
        <triangle>
          <v1>22903</v1>
          <v2>22902</v2>
          <v3>22885</v3>
        </triangle>
        <triangle>
          <v1>22904</v1>
          <v2>22886</v2>
          <v3>22887</v3>
        </triangle>
        <triangle>
          <v1>22904</v1>
          <v2>22903</v2>
          <v3>22886</v3>
        </triangle>
        <triangle>
          <v1>22905</v1>
          <v2>22887</v2>
          <v3>22888</v3>
        </triangle>
        <triangle>
          <v1>22905</v1>
          <v2>22904</v2>
          <v3>22887</v3>
        </triangle>
        <triangle>
          <v1>22891</v1>
          <v2>22905</v2>
          <v3>22888</v3>
        </triangle>
        <triangle>
          <v1>22906</v1>
          <v2>22891</v2>
          <v3>22892</v3>
        </triangle>
        <triangle>
          <v1>22906</v1>
          <v2>22892</v2>
          <v3>22907</v3>
        </triangle>
        <triangle>
          <v1>22908</v1>
          <v2>22907</v2>
          <v3>22892</v3>
        </triangle>
        <triangle>
          <v1>22908</v1>
          <v2>22892</v2>
          <v3>22893</v3>
        </triangle>
        <triangle>
          <v1>22898</v1>
          <v2>22897</v2>
          <v3>22896</v3>
        </triangle>
        <triangle>
          <v1>22909</v1>
          <v2>22896</v2>
          <v3>22895</v3>
        </triangle>
        <triangle>
          <v1>22910</v1>
          <v2>22911</v2>
          <v3>22896</v3>
        </triangle>
        <triangle>
          <v1>22912</v1>
          <v2>22896</v2>
          <v3>22911</v3>
        </triangle>
        <triangle>
          <v1>22909</v1>
          <v2>22910</v2>
          <v3>22896</v3>
        </triangle>
        <triangle>
          <v1>22898</v1>
          <v2>22896</v2>
          <v3>22912</v3>
        </triangle>
        <triangle>
          <v1>22913</v1>
          <v2>22895</v2>
          <v3>22899</v3>
        </triangle>
        <triangle>
          <v1>22913</v1>
          <v2>22909</v2>
          <v3>22895</v3>
        </triangle>
        <triangle>
          <v1>22914</v1>
          <v2>22899</v2>
          <v3>22900</v3>
        </triangle>
        <triangle>
          <v1>22914</v1>
          <v2>22913</v2>
          <v3>22899</v3>
        </triangle>
        <triangle>
          <v1>22915</v1>
          <v2>22900</v2>
          <v3>22901</v3>
        </triangle>
        <triangle>
          <v1>22915</v1>
          <v2>22914</v2>
          <v3>22900</v3>
        </triangle>
        <triangle>
          <v1>22916</v1>
          <v2>22901</v2>
          <v3>22902</v3>
        </triangle>
        <triangle>
          <v1>22916</v1>
          <v2>22915</v2>
          <v3>22901</v3>
        </triangle>
        <triangle>
          <v1>22917</v1>
          <v2>22902</v2>
          <v3>22903</v3>
        </triangle>
        <triangle>
          <v1>22917</v1>
          <v2>22916</v2>
          <v3>22902</v3>
        </triangle>
        <triangle>
          <v1>22918</v1>
          <v2>22903</v2>
          <v3>22904</v3>
        </triangle>
        <triangle>
          <v1>22918</v1>
          <v2>22917</v2>
          <v3>22903</v3>
        </triangle>
        <triangle>
          <v1>22919</v1>
          <v2>22904</v2>
          <v3>22905</v3>
        </triangle>
        <triangle>
          <v1>22919</v1>
          <v2>22918</v2>
          <v3>22904</v3>
        </triangle>
        <triangle>
          <v1>22920</v1>
          <v2>22905</v2>
          <v3>22891</v3>
        </triangle>
        <triangle>
          <v1>22920</v1>
          <v2>22919</v2>
          <v3>22905</v3>
        </triangle>
        <triangle>
          <v1>22906</v1>
          <v2>22920</v2>
          <v3>22891</v3>
        </triangle>
        <triangle>
          <v1>22921</v1>
          <v2>22906</v2>
          <v3>22907</v3>
        </triangle>
        <triangle>
          <v1>22922</v1>
          <v2>22921</v2>
          <v3>22907</v3>
        </triangle>
        <triangle>
          <v1>22922</v1>
          <v2>22907</v2>
          <v3>22908</v3>
        </triangle>
        <triangle>
          <v1>22923</v1>
          <v2>22911</v2>
          <v3>22910</v3>
        </triangle>
        <triangle>
          <v1>22912</v1>
          <v2>22911</v2>
          <v3>22923</v3>
        </triangle>
        <triangle>
          <v1>22924</v1>
          <v2>22910</v2>
          <v3>22909</v3>
        </triangle>
        <triangle>
          <v1>22925</v1>
          <v2>22926</v2>
          <v3>22910</v3>
        </triangle>
        <triangle>
          <v1>22923</v1>
          <v2>22910</v2>
          <v3>22926</v3>
        </triangle>
        <triangle>
          <v1>22924</v1>
          <v2>22925</v2>
          <v3>22910</v3>
        </triangle>
        <triangle>
          <v1>22927</v1>
          <v2>22909</v2>
          <v3>22913</v3>
        </triangle>
        <triangle>
          <v1>22927</v1>
          <v2>22924</v2>
          <v3>22909</v3>
        </triangle>
        <triangle>
          <v1>22928</v1>
          <v2>22913</v2>
          <v3>22914</v3>
        </triangle>
        <triangle>
          <v1>22928</v1>
          <v2>22927</v2>
          <v3>22913</v3>
        </triangle>
        <triangle>
          <v1>22929</v1>
          <v2>22914</v2>
          <v3>22915</v3>
        </triangle>
        <triangle>
          <v1>22929</v1>
          <v2>22928</v2>
          <v3>22914</v3>
        </triangle>
        <triangle>
          <v1>22930</v1>
          <v2>22915</v2>
          <v3>22916</v3>
        </triangle>
        <triangle>
          <v1>22930</v1>
          <v2>22929</v2>
          <v3>22915</v3>
        </triangle>
        <triangle>
          <v1>22931</v1>
          <v2>22916</v2>
          <v3>22917</v3>
        </triangle>
        <triangle>
          <v1>22931</v1>
          <v2>22930</v2>
          <v3>22916</v3>
        </triangle>
        <triangle>
          <v1>22932</v1>
          <v2>22917</v2>
          <v3>22918</v3>
        </triangle>
        <triangle>
          <v1>22932</v1>
          <v2>22931</v2>
          <v3>22917</v3>
        </triangle>
        <triangle>
          <v1>22933</v1>
          <v2>22918</v2>
          <v3>22919</v3>
        </triangle>
        <triangle>
          <v1>22933</v1>
          <v2>22932</v2>
          <v3>22918</v3>
        </triangle>
        <triangle>
          <v1>22934</v1>
          <v2>22919</v2>
          <v3>22920</v3>
        </triangle>
        <triangle>
          <v1>22934</v1>
          <v2>22933</v2>
          <v3>22919</v3>
        </triangle>
        <triangle>
          <v1>22935</v1>
          <v2>22920</v2>
          <v3>22906</v3>
        </triangle>
        <triangle>
          <v1>22935</v1>
          <v2>22934</v2>
          <v3>22920</v3>
        </triangle>
        <triangle>
          <v1>22935</v1>
          <v2>22906</v2>
          <v3>22921</v3>
        </triangle>
        <triangle>
          <v1>22935</v1>
          <v2>22921</v2>
          <v3>22936</v3>
        </triangle>
        <triangle>
          <v1>22922</v1>
          <v2>22936</v2>
          <v3>22921</v3>
        </triangle>
        <triangle>
          <v1>22937</v1>
          <v2>22935</v2>
          <v3>22936</v3>
        </triangle>
        <triangle>
          <v1>22937</v1>
          <v2>22936</v2>
          <v3>22938</v3>
        </triangle>
        <triangle>
          <v1>22939</v1>
          <v2>22938</v2>
          <v3>22936</v3>
        </triangle>
        <triangle>
          <v1>22939</v1>
          <v2>22936</v2>
          <v3>22922</v3>
        </triangle>
        <triangle>
          <v1>22940</v1>
          <v2>22926</v2>
          <v3>22925</v3>
        </triangle>
        <triangle>
          <v1>22923</v1>
          <v2>22926</v2>
          <v3>22940</v3>
        </triangle>
        <triangle>
          <v1>22941</v1>
          <v2>22925</v2>
          <v3>22924</v3>
        </triangle>
        <triangle>
          <v1>22942</v1>
          <v2>22943</v2>
          <v3>22925</v3>
        </triangle>
        <triangle>
          <v1>22940</v1>
          <v2>22925</v2>
          <v3>22943</v3>
        </triangle>
        <triangle>
          <v1>22941</v1>
          <v2>22942</v2>
          <v3>22925</v3>
        </triangle>
        <triangle>
          <v1>22944</v1>
          <v2>22924</v2>
          <v3>22927</v3>
        </triangle>
        <triangle>
          <v1>22944</v1>
          <v2>22941</v2>
          <v3>22924</v3>
        </triangle>
        <triangle>
          <v1>22945</v1>
          <v2>22927</v2>
          <v3>22928</v3>
        </triangle>
        <triangle>
          <v1>22945</v1>
          <v2>22944</v2>
          <v3>22927</v3>
        </triangle>
        <triangle>
          <v1>22946</v1>
          <v2>22928</v2>
          <v3>22929</v3>
        </triangle>
        <triangle>
          <v1>22946</v1>
          <v2>22945</v2>
          <v3>22928</v3>
        </triangle>
        <triangle>
          <v1>22947</v1>
          <v2>22929</v2>
          <v3>22930</v3>
        </triangle>
        <triangle>
          <v1>22947</v1>
          <v2>22946</v2>
          <v3>22929</v3>
        </triangle>
        <triangle>
          <v1>22948</v1>
          <v2>22930</v2>
          <v3>22931</v3>
        </triangle>
        <triangle>
          <v1>22948</v1>
          <v2>22947</v2>
          <v3>22930</v3>
        </triangle>
        <triangle>
          <v1>22949</v1>
          <v2>22931</v2>
          <v3>22932</v3>
        </triangle>
        <triangle>
          <v1>22949</v1>
          <v2>22948</v2>
          <v3>22931</v3>
        </triangle>
        <triangle>
          <v1>22950</v1>
          <v2>22932</v2>
          <v3>22933</v3>
        </triangle>
        <triangle>
          <v1>22950</v1>
          <v2>22949</v2>
          <v3>22932</v3>
        </triangle>
        <triangle>
          <v1>22951</v1>
          <v2>22933</v2>
          <v3>22934</v3>
        </triangle>
        <triangle>
          <v1>22951</v1>
          <v2>22950</v2>
          <v3>22933</v3>
        </triangle>
        <triangle>
          <v1>22952</v1>
          <v2>22934</v2>
          <v3>22935</v3>
        </triangle>
        <triangle>
          <v1>22952</v1>
          <v2>22951</v2>
          <v3>22934</v3>
        </triangle>
        <triangle>
          <v1>22937</v1>
          <v2>22952</v2>
          <v3>22935</v3>
        </triangle>
        <triangle>
          <v1>22953</v1>
          <v2>22937</v2>
          <v3>22938</v3>
        </triangle>
        <triangle>
          <v1>22953</v1>
          <v2>22938</v2>
          <v3>22954</v3>
        </triangle>
        <triangle>
          <v1>22955</v1>
          <v2>22954</v2>
          <v3>22938</v3>
        </triangle>
        <triangle>
          <v1>22955</v1>
          <v2>22938</v2>
          <v3>22939</v3>
        </triangle>
        <triangle>
          <v1>22956</v1>
          <v2>22943</v2>
          <v3>22942</v3>
        </triangle>
        <triangle>
          <v1>22940</v1>
          <v2>22943</v2>
          <v3>22956</v3>
        </triangle>
        <triangle>
          <v1>22957</v1>
          <v2>22942</v2>
          <v3>22941</v3>
        </triangle>
        <triangle>
          <v1>22957</v1>
          <v2>22958</v2>
          <v3>22942</v3>
        </triangle>
        <triangle>
          <v1>22956</v1>
          <v2>22942</v2>
          <v3>22958</v3>
        </triangle>
        <triangle>
          <v1>22959</v1>
          <v2>22941</v2>
          <v3>22944</v3>
        </triangle>
        <triangle>
          <v1>22959</v1>
          <v2>22957</v2>
          <v3>22941</v3>
        </triangle>
        <triangle>
          <v1>22960</v1>
          <v2>22944</v2>
          <v3>22945</v3>
        </triangle>
        <triangle>
          <v1>22960</v1>
          <v2>22959</v2>
          <v3>22944</v3>
        </triangle>
        <triangle>
          <v1>22961</v1>
          <v2>22945</v2>
          <v3>22946</v3>
        </triangle>
        <triangle>
          <v1>22961</v1>
          <v2>22960</v2>
          <v3>22945</v3>
        </triangle>
        <triangle>
          <v1>22962</v1>
          <v2>22946</v2>
          <v3>22947</v3>
        </triangle>
        <triangle>
          <v1>22962</v1>
          <v2>22961</v2>
          <v3>22946</v3>
        </triangle>
        <triangle>
          <v1>22963</v1>
          <v2>22947</v2>
          <v3>22948</v3>
        </triangle>
        <triangle>
          <v1>22963</v1>
          <v2>22962</v2>
          <v3>22947</v3>
        </triangle>
        <triangle>
          <v1>22964</v1>
          <v2>22948</v2>
          <v3>22949</v3>
        </triangle>
        <triangle>
          <v1>22964</v1>
          <v2>22963</v2>
          <v3>22948</v3>
        </triangle>
        <triangle>
          <v1>22965</v1>
          <v2>22949</v2>
          