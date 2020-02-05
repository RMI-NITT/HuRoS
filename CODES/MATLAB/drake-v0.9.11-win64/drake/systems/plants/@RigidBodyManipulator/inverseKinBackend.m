function varargout = inverseKinBackend(obj,mode,t,q_seed,q_nom,varargin)
% inverseKin(obj,mode,q_seed,q_nom,t,constraint1,constraint2,...,options)
% Two modes,
% mode 1 -- solve IK
%   min_q (q-q_nom'*Q*(q-q_nom)
%   subject to
%          constraint1 at t
%          constraint2 at t
%   .....
%   using q_seed as the initial guess
%   If t = [], then all kc are ative.
%   If t is a scalar, then solve IK for that single time only
%   If t is an array, then for each time, solve an IK
%
% mode 2 -- solve IK for a sequence of time
%   min_qi sum_i qdd_i'*Qa*qdd_i+qd_i'*Qv*qd_i+(q_i-q_nom(:,i))'*Q*(q_i-_nom(:,i))
%   subject to
%          constraint_j at t_i
%   qd,qdd are computed by supposing q is a cubic spline.
% @param q_seed       The seed guess, a matrix, wich column i representing the
%                     seed at t[i]
% @param q_nom        The nominal posture, a matrix, with column i representing
%                     The nominal posture at t[i]
% @param constraint   A Constraint class object, accept SingleTimeKinematicConstraint,
%                     MultipleTimeKinematicConstraint,
%                     QuasiStaticConstraint, PostureConstraint and MultipleTimeLinearPostureConstraint
%                     currently
% @param ikoptions    An IKoptions object, please refer to IKoptions for detail

% note: keeping typecheck/sizecheck to a minimum because this might have to
% run inside a dynamical system (so should be fast)
if(~checkDependency('snopt') && ~checkDependency('studentsnopt'))
  error('Drake:MissingDependency:SNOPT','inverseKinBackend requires SNOPT. Either try install SNOPT, or try using InverseKinematics (capital case) or InverseKinematicsTrajectory (capital case) and set solver to fmincon or ipopt');
end

global SNOPT_USERFUN;
nq = obj.getNumPositions();
if(isempty(t))
  nT = 1;
else
  nT = length(t);
end
if(~isa(varargin{end},'IKoptions'))
  varargin{end+1} = IKoptions(obj);
end
ikoptions = varargin{end};
%   qsc = QuasiStaticConstraint(obj);
qsc = QuasiStaticConstraint(obj,[-inf,inf],1);
qsc = qsc.setActive(false);
[joint_min,joint_max] = obj.getJointLimits();
joint_min = bsxfun(@times,joint_min,ones(1,nT));
joint_max = bsxfun(@times,joint_max,ones(1,nT));
for i = 1:nT
  if(isempty(t))
    ti = [];
  else
    ti = t(i);
  end
end
st_kc_cell = cell(1,length(varargin)-1); % SingleTimeKinematicConstraint
mt_kc_cell = cell(1,length(varargin)-1); % MultipleTimeKinematicConstraint
mtlpc_cell = cell(1,length(varargin)-1); % MultipleTimeLinearPostureConstraint
stlpc_cell = cell(1,length(varargin)-1); % SingleTimeLinearPostureConstraint
num_st_kc = 0;
num_mt_kc = 0;
num_mtlpc = 0;
num_stlpc = 0;
for j = 1:length(varargin)-1
  if(isa(varargin{j},'RigidBodyConstraint'))
    if(isa(varargin{j},'PostureConstraint'))
      for i = 1:nT
        if(isempty(t))
          ti = [];
        else
          ti = t(i);
        end
        [lb,ub] = varargin{j}.bounds(ti);
        joint_min(:,i) = max([joint_min(:,i) lb],[],2);
        joint_max(:,i) = min([joint_max(:,i) ub],[],2);
        if(any(joint_min(:,i)>joint_max(:,i)))
          error('Joint maximum should be no smaller than joint minimum, check if the joint limits are consistent');
        end
      end
    elseif(isa(varargin{j},'SingleTimeKinematicConstraint'))
      num_st_kc = num_st_kc+1;
      st_kc_cell{num_st_kc} = varargin{j};
    elseif(isa(varargin{j},'MultipleTimeKinematicConstraint'))
      num_mt_kc = num_mt_kc+1;
      mt_kc_cell{num_mt_kc} = varargin{j};
    elseif(isa(varargin{j},'QuasiStaticConstraint'))
      qsc = varargin{j};
    elseif(isa(varargin{j},'MultipleTimeLinearPostureConstraint'))
      num_mtlpc = num_mtlpc+1;
      mtlpc_cell{num_mtlpc} = varargin{j};
    elseif(isa(varargin{j},'SingleTimeLinearPostureConstraint'))
      num_stlpc = num_stlpc+1;
      stlpc_cell{num_stlpc} = varargin{j};
    end
  end
end

st_kc_cell = st_kc_cell(1:num_st_kc);
mt_kc_cell = mt_kc_cell(1:num_mt_kc);
mtlpc_cell = mtlpc_cell(1:num_mtlpc);
stlpc_cell = stlpc_cell(1:num_stlpc);
Q = ikoptions.Q;
debug_mode = ikoptions.debug_mode;
if(isempty(qsc))
  qscActiveFlag = false;
else
  qscActiveFlag = qsc.active;
end
sizecheck(q_seed,[nq,nT]);
sizecheck(q_nom,[nq,nT]);
q = zeros(nq,nT);
iCfun_cell = cell(1,nT);
jCvar_cell = cell(1,nT);
nc_array = zeros(1,nT); % the number of constraint in each knot
nG_array = zeros(1,nT); % the number of G in each knot
nA_array = zeros(1,nT); % the number of A in each knot
A_cell = cell(1,nT);
iAfun_cell = cell(1,nT);
jAvar_cell = cell(1,nT);
Cmin_cell = cell(1,nT);
Cmax_cell = cell(1,nT);
Cname_cell = cell(1,nT);
infeasible_constraint = {};
if(mode == 1)
  info = zeros(1,nT);
elseif(mode == 2)
  info = 0;
end
for i = 1:nT
  if(isempty(t))
    ti = [];
  else
    ti = t(i);
  end
  for j = 1:length(st_kc_cell)
    stkc = st_kc_cell{j};
    [lb,ub] = stkc.bounds(ti);
    Cmin_cell{i} = [Cmin_cell{i};lb];
    Cmax_cell{i} = [Cmax_cell{i};ub];
    nc = stkc.getNumConstraint(ti);
    iCfun_cell{i} = [iCfun_cell{i};nc_array(i)+reshape(bsxfun(@times,(1:nc)',ones(1,nq)),[],1)];
    jCvar_cell{i} = [jCvar_cell{i};reshape(bsxfun(@times,(1:nq),ones(nc,1)),[],1)];
    nc_array(i) = nc_array(i)+nc;
    nG_array(i) = nG_array(i)+nc*nq;
    if(debug_mode)
      Cname_cell{i} = [Cname_cell{i};stkc.name(ti)];
    end
  end
  stlpc_nc = zeros(1,num_stlpc);
  for j = 1:length(stlpc_cell)
    stlpc = stlpc_cell{j};
    [lb,ub] = stlpc.bounds(ti);
    Cmin_cell{i} = [Cmin_cell{i};lb];
    Cmax_cell{i} = [Cmax_cell{i};ub];
    stlpc_nc(j) = stlpc.getNumConstraint(ti);
    [iAfun,jAvar,A] = stlpc.geval(ti);
    iAfun_cell{i} = [iAfun_cell{i};nc_array(i)+iAfun];
    jAvar_cell{i} = [jAvar_cell{i};jAvar];
    A_cell{i} = [A_cell{i};A];
    nc_array(i) = nc_array(i)+stlpc_nc(j);
    nA_array(i) = nA_array(i)+length(A);
    if(debug_mode)
      Cname_cell{i} = [Cname_cell{i};stlpc.name(ti)];
    end
  end
  if(qscActiveFlag)
    num_qsc_cnst = qsc.getNumConstraint(ti);
    iCfun_cell{i} = [iCfun_cell{i}; nc_array(i)+reshape(bsxfun(@times,(1:(num_qsc_cnst-1))',ones(1,nq+qsc.num_pts)),[],1)];
    jCvar_cell{i} = [jCvar_cell{i}; reshape(bsxfun(@times,ones(num_qsc_cnst-1,1),(1:(nq+qsc.num_pts))),[],1)];
    iAfun_cell{i} = [iAfun_cell{i}; nc_array(i)+num_qsc_cnst*ones(qsc.num_pts,1)];
    jAvar_cell{i} = [jAvar_cell{i}; nq+(1:qsc.num_pts)'];
    A_cell{i} = [A_cell{i};ones(qsc.num_pts,1)];
    [qsc_lb,qsc_ub] = qsc.bounds(ti);
    Cmin_cell{i} = [Cmin_cell{i};[qsc_lb;1]];
    Cmax_cell{i} = [Cmax_cell{i};[qsc_ub;1]];
    nc_array(i) = nc_array(i)+num_qsc_cnst;
    nG_array(i) = nG_array(i)+(num_qsc_cnst-1)*(nq+qsc.num_pts);
    nA_array(i) = nA_array(i)+qsc.num_pts;
    if(debug_mode)
      Cname_cell{i} = [Cname_cell{i};qsc.name(ti);{sprintf('quasi static constraint weights at time %10.4f',ti)}];
    end
  end
  if(mode == 1)
    xlow = joint_min;
    xupp = joint_max;
    if(~ikoptions.sequentialSeedFlag)
      x0 = q_seed(:,i);
    else
      if(i == 1)
        x0 = q_seed(:,i);
      else
        if(info(i-1)>10)
          x0 = q_seed(:,i);
        else
          x0 = q(:,i-1);
        end
      end
    end
    if(qscActiveFlag)
      xlow = [xlow;zeros(qsc.num_pts,nT)];
      xupp = [xupp;ones(qsc.num_pts,nT)];
      x0 = [x0; 1/qsc.num_pts*ones(qsc.num_pts,1)];
    end
    if(any(xlow>xupp))
      error('Drake:inverseKinBackend: The lower bound of the decision variable is larger than the upper bound, possibly check the conflicting posture constraint');
    end
    SNOPT_USERFUN = @(w) IK_userfun(obj,w,ti,st_kc_cell,stlpc_nc,q_nom(:,i),Q,nq,nc_array(i),nG_array(i),qsc);
    snseti('Major iterations limit',ikoptions.SNOPT_MajorIterationsLimit);
    snseti('Iterations limit',ikoptions.SNOPT_IterationsLimit);
    snseti('Superbasics limit',ikoptions.SNOPT_SuperbasicsLimit);
    snsetr('Major optimality tolerance',ikoptions.SNOPT_MajorOptimalityTolerance);
    snsetr('Major feasibility tolerance',ikoptions.SNOPT_MajorFeasibilityTolerance);
    A = A_cell{i};
    iAfun = iAfun_cell{i}+1;
    jAvar = jAvar_cell{i};
    iGfun = [ones(nq,1);iCfun_cell{i}+1];
    jGvar = [(1:nq)';jCvar_cell{i}];
    Flow = [-inf;Cmin_cell{i}];
    Fupp = [inf;Cmax_cell{i}];
    [x_sol,F,info(i)] = snopt(x0,xlow(:,i),xupp(:,i),Flow,Fupp,'snoptUserfun',0,1,A,iAfun,jAvar,iGfun,jGvar);
    q(:,i) = x_sol(1:nq);
    if(debug_mode)
      Fname = [{'objective'};Cname_cell{i}];
    end

    if(info(i) == 13 || info(i) == 31 || info(i) == 32)
      ub_err = F-Fupp;
      max_ub_error = max(ub_err);
      max_ub_error = max_ub_error*(max_ub_error>0);
      lb_err = Flow-F;
      max_lb_error = max(lb_err);
      max_lb_error = max_lb_error*(max_lb_error>0);
      if(max_ub_error+max_lb_error>1e-4)
        infeasible_constraint_idx = (ub_err>5e-5)|(lb_err>5e-5);
        if(debug_mode)
          infeasible_constraint = [infeasible_constraint Fname(infeasible_constraint_idx)];
        end
      elseif(info(i) == 13)
        info(i) = 4;
      elseif(info(i) == 31)
        info(i) = 5;
      elseif(info(i) == 32)
        info(i) = 6;
      end
    end
  end
end
if(mode == 1)
  success_idx = info<10;
  q(:,success_idx) = reshape(max([reshape(q(:,success_idx),[],1) reshape(joint_min(:,success_idx),[],1)],[],2),nq,[]);
  q(:,success_idx) = reshape(min([reshape(q(:,success_idx),[],1) reshape(joint_max(:,success_idx),[],1)],[],2),nq,[]);
  varargout{1} = q;
  varargout{2} = info;
  if(nargout == 3)
    varargout{3} = infeasible_constraint;
  end
end
if(mode == 2)
  Qa = ikoptions.Qa;
  Qv = ikoptions.Qv;

  dt = diff(t);
  dt_ratio = dt(1:end-1)./dt(2:end);
  if(ikoptions.fixInitialState)
    q0_fixed = q_seed(:,1);
    qdot0_fixed = (ikoptions.qd0_lb+ikoptions.qd0_ub)/2;
    qstart_idx = 2; % q(1) is not a decision variable
    num_qfree = nT-1; % Number of q being decision variables
    num_qdotfree = 1; % Number of qdot being decision variables
  else
    q0_fixed = [];
    qdot0_fixed = [];
    qstart_idx = 1;
    num_qfree = nT;
    num_qdotfree = 2;
  end
  % Suppose the joint angles are interpolated using cubic splines, then the
  %[qdot(2);qdot(3);...;qdot(nT-1)] = velocity_mat*[q(1);q(2);...;q(nT)]+velocity_mat_qd0*qdot(1)+velocity_mat_qdf*qdot(nT);
  velocity_mat1_diag1 = reshape([ones(nq,1) repmat(dt(1:end-1).*(2+2*dt_ratio),nq,1) ones(nq,1)],[],1);
  velocity_mat1_diag2 = reshape([zeros(nq,1) repmat(dt(1:end-1).*dt_ratio,nq,1)],[],1);
  velocity_mat1_diag3 = [reshape(repmat(dt(1:end-1),nq,1),[],1);zeros(nq,1)];
  velocity_mat1 = sparse((1:nq*nT)',(1:nq*nT)',velocity_mat1_diag1)...
      +sparse((1:nq*(nT-1))',nq+(1:nq*(nT-1))',velocity_mat1_diag2,nq*nT,nq*nT)...
      +sparse(nq+(1:nq*(nT-1))',(1:nq*(nT-1))',velocity_mat1_diag3,nq*nT,nq*nT);


  velocity_mat2_diag1 = reshape([zeros(nq,1) bsxfun(@times,3*ones(1,nT-2)-3*dt_ratio.^2,ones(nq,1)) zeros(nq,1)],[],1);
  velocity_mat2_diag2 = reshape([zeros(nq,1) bsxfun(@times,3*dt_ratio.^2,ones(nq,1))],[],1);
  velocity_mat2_diag3 = [-3*ones(nq*(nT-2),1);zeros(nq,1)];
  velocity_mat2 = sparse((1:nq*nT)',(1:nq*nT)',velocity_mat2_diag1)...
      +sparse((1:nq*(nT-1))',nq+(1:nq*(nT-1))',velocity_mat2_diag2,nq*nT,nq*nT)...
      +sparse(nq+(1:nq*(nT-1))',(1:nq*(nT-1))',velocity_mat2_diag3,nq*nT,nq*nT);
  velocity_mat = velocity_mat1\velocity_mat2;
  velocity_mat = velocity_mat(nq+1:end-nq,:);

  velocity_mat_qd0 = -velocity_mat1(nq+1:nq*(nT-1),nq+1:nq*(nT-1))\velocity_mat1(nq+(1:nq*(nT-2)),1:nq);
  velocity_mat_qdf = -velocity_mat1(nq+1:nq*(nT-1),nq+1:nq*(nT-1))\velocity_mat1(nq+(1:nq*(nT-2)),nq*(nT-1)+(1:nq));

  % [qddot(1);...qddot(k)] =
  % accel_mat*[q(1);...;q(nT)]+accel_mat_qdot0*qdot(1)+accel_mat_qdf*qdot(nT)
  accel_mat1_diag1 = reshape(bsxfun(@times,[-6./(dt.^2) -6/(dt(end)^2)],ones(nq,1)),[],1);
  accel_mat1_diag2 = reshape(bsxfun(@times,6./(dt.^2),ones(nq,1)),[],1);
  accel_mat1_diag3 = 6/(dt(end)^2)*ones(nq,1);
  accel_mat1 = sparse((1:nq*nT)',(1:nq*nT)',accel_mat1_diag1)...
      +sparse((1:nq*(nT-1))',nq+(1:nq*(nT-1))',accel_mat1_diag2,nq*nT,nq*nT)...
      +sparse(nq*(nT-1)+(1:nq)',nq*(nT-2)+(1:nq)',accel_mat1_diag3,nq*nT,nq*nT);
  accel_mat2_diag1 = reshape(bsxfun(@times,[-4./dt 4/dt(end)],ones(nq,1)),[],1);
  accel_mat2_diag2 = reshape(bsxfun(@times,-2./dt,ones(nq,1)),[],1);
  accel_mat2_diag3 = 2/dt(end)*ones(nq,1);
  accel_mat2 = sparse((1:nq*nT)',(1:nq*nT)',a         <v2>25723</v2>
          <v3>38408</v3>
        </triangle>
        <triangle>
          <v1>38413</v1>
          <v2>38409</v2>
          <v3>38410</v3>
        </triangle>
        <triangle>
          <v1>38413</v1>
          <v2>38412</v2>
          <v3>38409</v3>
        </triangle>
        <triangle>
          <v1>38414</v1>
          <v2>38413</v2>
          <v3>38410</v3>
        </triangle>
        <triangle>
          <v1>38415</v1>
          <v2>38414</v2>
          <v3>38410</v3>
        </triangle>
        <triangle>
          <v1>38415</v1>
          <v2>38410</v2>
          <v3>38411</v3>
        </triangle>
        <triangle>
          <v1>25729</v1>
          <v2>25723</v2>
          <v3>38412</v3>
        </triangle>
        <triangle>
          <v1>38416</v1>
          <v2>38412</v2>
          <v3>38413</v3>
        </triangle>
        <triangle>
          <v1>38416</v1>
          <v2>25729</v2>
          <v3>38412</v3>
        </triangle>
        <triangle>
          <v1>38417</v1>
          <v2>38413</v2>
          <v3>38414</v3>
        </triangle>
        <triangle>
          <v1>38417</v1>
          <v2>38416</v2>
          <v3>38413</v3>
        </triangle>
        <triangle>
          <v1>38418</v1>
          <v2>38417</v2>
          <v3>38414</v3>
        </triangle>
        <triangle>
          <v1>38419</v1>
          <v2>38418</v2>
          <v3>38414</v3>
        </triangle>
        <triangle>
          <v1>38419</v1>
          <v2>38414</v2>
          <v3>38415</v3>
        </triangle>
        <triangle>
          <v1>25735</v1>
          <v2>25729</v2>
          <v3>38416</v3>
        </triangle>
        <triangle>
          <v1>38420</v1>
          <v2>38416</v2>
          <v3>38417</v3>
        </triangle>
        <triangle>
          <v1>38420</v1>
          <v2>25735</v2>
          <v3>38416</v3>
        </triangle>
        <triangle>
          <v1>38421</v1>
          <v2>38417</v2>
          <v3>38418</v3>
        </triangle>
        <triangle>
          <v1>38421</v1>
          <v2>38420</v2>
          <v3>38417</v3>
        </triangle>
        <triangle>
          <v1>38422</v1>
          <v2>38421</v2>
          <v3>38418</v3>
        </triangle>
        <triangle>
          <v1>38423</v1>
          <v2>38422</v2>
          <v3>38418</v3>
        </triangle>
        <triangle>
          <v1>38423</v1>
          <v2>38418</v2>
          <v3>38419</v3>
        </triangle>
        <triangle>
          <v1>25741</v1>
          <v2>25735</v2>
          <v3>38420</v3>
        </triangle>
        <triangle>
          <v1>38424</v1>
          <v2>38420</v2>
          <v3>38421</v3>
        </triangle>
        <triangle>
          <v1>38424</v1>
          <v2>25741</v2>
          <v3>38420</v3>
        </triangle>
        <triangle>
          <v1>38425</v1>
          <v2>38421</v2>
          <v3>38422</v3>
        </triangle>
        <triangle>
          <v1>38425</v1>
          <v2>38424</v2>
          <v3>38421</v3>
        </triangle>
        <triangle>
          <v1>38426</v1>
          <v2>38425</v2>
          <v3>38422</v3>
        </triangle>
        <triangle>
          <v1>38427</v1>
          <v2>38426</v2>
          <v3>38422</v3>
        </triangle>
        <triangle>
          <v1>38427</v1>
          <v2>38422</v2>
          <v3>38423</v3>
        </triangle>
        <triangle>
          <v1>25747</v1>
          <v2>25741</v2>
          <v3>38424</v3>
        </triangle>
        <triangle>
          <v1>38428</v1>
          <v2>38424</v2>
          <v3>38425</v3>
        </triangle>
        <triangle>
          <v1>38428</v1>
          <v2>25747</v2>
          <v3>38424</v3>
        </triangle>
        <triangle>
          <v1>38429</v1>
          <v2>38425</v2>
          <v3>38426</v3>
        </triangle>
        <triangle>
          <v1>38429</v1>
          <v2>38428</v2>
          <v3>38425</v3>
        </triangle>
        <triangle>
          <v1>38430</v1>
          <v2>38429</v2>
          <v3>38426</v3>
        </triangle>
        <triangle>
          <v1>38431</v1>
          <v2>38430</v2>
          <v3>38426</v3>
        </triangle>
        <triangle>
          <v1>38431</v1>
          <v2>38426</v2>
          <v3>38427</v3>
        </triangle>
        <triangle>
          <v1>25753</v1>
          <v2>25747</v2>
          <v3>38428</v3>
        </triangle>
        <triangle>
          <v1>38432</v1>
          <v2>38428</v2>
          <v3>38429</v3>
        </triangle>
        <triangle>
          <v1>38432</v1>
          <v2>25753</v2>
          <v3>38428</v3>
        </triangle>
        <triangle>
          <v1>38433</v1>
          <v2>38429</v2>
          <v3>38430</v3>
        </triangle>
        <triangle>
          <v1>38433</v1>
          <v2>38432</v2>
          <v3>38429</v3>
        </triangle>
        <triangle>
          <v1>38434</v1>
          <v2>38433</v2>
          <v3>38430</v3>
        </triangle>
        <triangle>
          <v1>38435</v1>
          <v2>38434</v2>
          <v3>38430</v3>
        </triangle>
        <triangle>
          <v1>38435</v1>
          <v2>38430</v2>
          <v3>38431</v3>
        </triangle>
        <triangle>
          <v1>25759</v1>
          <v2>25753</v2>
          <v3>38432</v3>
        </triangle>
        <triangle>
          <v1>38436</v1>
          <v2>38432</v2>
          <v3>38433</v3>
        </triangle>
        <triangle>
          <v1>38436</v1>
          <v2>25759</v2>
          <v3>38432</v3>
        </triangle>
        <triangle>
          <v1>38437</v1>
          <v2>38433</v2>
          <v3>38434</v3>
        </triangle>
        <triangle>
          <v1>38437</v1>
          <v2>38436</v2>
          <v3>38433</v3>
        </triangle>
        <triangle>
          <v1>38438</v1>
          <v2>38437</v2>
          <v3>38434</v3>
        </triangle>
        <triangle>
          <v1>38439</v1>
          <v2>38438</v2>
          <v3>38434</v3>
        </triangle>
        <triangle>
          <v1>38439</v1>
          <v2>38434</v2>
          <v3>38435</v3>
        </triangle>
        <triangle>
          <v1>25765</v1>
          <v2>25759</v2>
          <v3>38436</v3>
        </triangle>
        <triangle>
          <v1>38440</v1>
          <v2>38436</v2>
          <v3>38437</v3>
        </triangle>
        <triangle>
          <v1>38440</v1>
          <v2>25765</v2>
          <v3>38436</v3>
        </triangle>
        <triangle>
          <v1>38441</v1>
          <v2>38437</v2>
          <v3>38438</v3>
        </triangle>
        <triangle>
          <v1>38441</v1>
          <v2>38440</v2>
          <v3>38437</v3>
        </triangle>
        <triangle>
          <v1>38442</v1>
          <v2>38441</v2>
          <v3>38438</v3>
        </triangle>
        <triangle>
          <v1>38443</v1>
          <v2>38442</v2>
          <v3>38438</v3>
        </triangle>
        <triangle>
          <v1>38443</v1>
          <v2>38438</v2>
          <v3>38439</v3>
        </triangle>
        <triangle>
          <v1>25771</v1>
          <v2>25765</v2>
          <v3>38440</v3>
        </triangle>
        <triangle>
          <v1>38444</v1>
          <v2>38440</v2>
          <v3>38441</v3>
        </triangle>
        <triangle>
          <v1>38444</v1>
          <v2>25771</v2>
          <v3>38440</v3>
        </triangle>
        <triangle>
          <v1>38445</v1>
          <v2>38441</v2>
          <v3>38442</v3>
        </triangle>
        <triangle>
          <v1>38445</v1>
          <v2>38444</v2>
          <v3>38441</v3>
        </triangle>
        <triangle>
          <v1>38446</v1>
          <v2>38445</v2>
          <v3>38442</v3>
        </triangle>
        <triangle>
          <v1>38447</v1>
          <v2>38446</v2>
          <v3>38442</v3>
        </triangle>
        <triangle>
          <v1>38447</v1>
          <v2>38442</v2>
          <v3>38443</v3>
        </triangle>
        <triangle>
          <v1>25777</v1>
          <v2>25771</v2>
          <v3>38444</v3>
        </triangle>
        <triangle>
          <v1>38448</v1>
          <v2>38444</v2>
          <v3>38445</v3>
        </triangle>
        <triangle>
          <v1>38448</v1>
          <v2>25777</v2>
          <v3>38444</v3>
        </triangle>
        <triangle>
          <v1>38449</v1>
          <v2>38445</v2>
          <v3>38446</v3>
        </triangle>
        <triangle>
          <v1>38449</v1>
          <v2>38448</v2>
          <v3>38445</v3>
        </triangle>
        <triangle>
          <v1>38450</v1>
          <v2>38449</v2>
          <v3>38446</v3>
        </triangle>
        <triangle>
          <v1>38451</v1>
          <v2>38450</v2>
          <v3>38446</v3>
        </triangle>
        <triangle>
          <v1>38451</v1>
          <v2>38446</v2>
          <v3>38447</v3>
        </triangle>
        <triangle>
          <v1>25783</v1>
          <v2>25777</v2>
          <v3>38448</v3>
        </triangle>
        <triangle>
          <v1>38452</v1>
          <v2>38448</v2>
          <v3>38449</v3>
        </triangle>
        <triangle>
          <v1>38452</v1>
          <v2>25783</v2>
          <v3>38448</v3>
        </triangle>
        <triangle>
          <v1>38453</v1>
          <v2>38449</v2>
          <v3>38450</v3>
        </triangle>
        <triangle>
          <v1>38453</v1>
          <v2>38452</v2>
          <v3>38449</v3>
        </triangle>
        <triangle>
          <v1>38454</v1>
          <v2>38453</v2>
          <v3>38450</v3>
        </triangle>
        <triangle>
          <v1>38455</v1>
          <v2>38454</v2>
          <v3>38450</v3>
        </triangle>
        <triangle>
          <v1>38455</v1>
          <v2>38450</v2>
          <v3>38451</v3>
        </triangle>
        <triangle>
          <v1>25789</v1>
          <v2>25783</v2>
          <v3>38452</v3>
        </triangle>
        <triangle>
          <v1>38456</v1>
          <v2>38452</v2>
          <v3>38453</v3>
        </triangle>
        <triangle>
          <v1>38456</v1>
          <v2>25789</v2>
          <v3>38452</v3>
        </triangle>
        <triangle>
          <v1>38457</v1>
          <v2>38453</v2>
          <v3>38454</v3>
        </triangle>
        <triangle>
          <v1>38457</v1>
          <v2>38456</v2>
          <v3>38453</v3>
        </triangle>
        <triangle>
          <v1>38458</v1>
          <v2>38457</v2>
          <v3>38454</v3>
        </triangle>
        <triangle>
          <v1>38459</v1>
          <v2>38458</v2>
          <v3>38454</v3>
        </triangle>
        <triangle>
          <v1>38459</v1>
          <v2>38454</v2>
          <v3>38455</v3>
        </triangle>
        <triangle>
          <v1>25795</v1>
          <v2>25789</v2>
          <v3>38456</v3>
        </triangle>
        <triangle>
          <v1>38460</v1>
          <v2>38456</v2>
          <v3>38457</v3>
        </triangle>
        <triangle>
          <v1>38460</v1>
          <v2>25795</v2>
          <v3>38456</v3>
        </triangle>
        <triangle>
          <v1>38461</v1>
          <v2>38457</v2>
          <v3>38458</v3>
        </triangle>
        <triangle>
          <v1>38461</v1>
          <v2>38460</v2>
          <v3>38457</v3>
        </triangle>
        <triangle>
          <v1>38462</v1>
          <v2>38461</v2>
          <v3>38458</v3>
        </triangle>
        <triangle>
          <v1>38463</v1>
          <v2>38462</v2>
          <v3>38458</v3>
        </triangle>
        <triangle>
          <v1>38463</v1>
          <v2>38458</v2>
          <v3>38459</v3>
        </triangle>
        <triangle>
          <v1>25801</v1>
          <v2>25795</v2>
          <v3>38460</v3>
        </triangle>
        <triangle>
          <v1>38464</v1>
          <v2>38460</v2>
          <v3>38461</v3>
        </triangle>
        <triangle>
          <v1>38464</v1>
          <v2>25801</v2>
          <v3>38460</v3>
        </triangle>
        <triangle>
          <v1>38465</v1>
          <v2>38461</v2>
          <v3>38462</v3>
        </triangle>
        <triangle>
          <v1>38465</v1>
          <v2>38464</v2>
          <v3>38461</v3>
        </triangle>
        <triangle>
          <v1>38466</v1>
          <v2>38465</v2>
          <v3>38462</v3>
        </triangle>
        <triangle>
          <v1>38467</v1>
          <v2>38466</v2>
          <v3>38462</v3>
        </triangle>
        <triangle>
          <v1>38467</v1>
          <v2>38462</v2>
          <v3>38463</v3>
        </triangle>
        <triangle>
          <v1>25807</v1>
          <v2>25801</v2>
          <v3>38464</v3>
        </triangle>
        <triangle>
          <v1>38468</v1>
          <v2>38464</v2>
          <v3>38465</v3>
        </triangle>
        <triangle>
          <v1>38468</v1>
          <v2>25807</v2>
          <v3>38464</v3>
        </triangle>
        <triangle>
          <v1>38469</v1>
          <v2>38465</v2>
          <v3>38466</v3>
        </triangle>
        <triangle>
          <v1>38469</v1>
          <v2>38468</v2>
          <v3>38465</v3>
        </triangle>
        <triangle>
          <v1>38470</v1>
          <v2>38469</v2>
          <v3>38466</v3>
        </triangle>
        <triangle>
          <v1>38471</v1>
          <v2>38470</v2>
          <v3>38466</v3>
        </triangle>
        <triangle>
          <v1>38471</v1>
          <v2>38466</v2>
          <v3>38467</v3>
        </triangle>
        <triangle>
          <v1>25813</v1>
          <v2>25807</v2>
          <v3>38468</v3>
        </triangle>
        <triangle>
          <v1>38472</v1>
          <v2>38468</v2>
          <v3>38469</v3>
        </triangle>
        <triangle>
          <v1>38472</v1>
          <v2>25813</v2>
          <v3>38468</v3>
        </triangle>
        <triangle>
          <v1>38473</v1>
          <v2>38469</v2>
          <v3>38470</v3>
        </triangle>
        <triangle>
          <v1>38473</v1>
          <v2>38472</v2>
          <v3>38469</v3>
        </triangle>
        <triangle>
          <v1>38474</v1>
          <v2>38473</v2>
          <v3>38470</v3>
        </triangle>
        <triangle>
          <v1>38475</v1>
          <v2>38474</v2>
          <v3>38470</v3>
        </triangle>
        <triangle>
          <v1>38475</v1>
          <v2>38470</v2>
          <v3>38471</v3>
        </triangle>
        <triangle>
          <v1>25819</v1>
          <v2>25813</v2>
          <v3>38472</v3>
        </triangle>
        <triangle>
          <v1>38476</v1>
          <v2>38472</v2>
          <v3>38473</v3>
        </triangle>
        <triangle>
          <v1>38476</v1>
          <v2>25819</v2>
          <v3>38472</v3>
        </triangle>
        <triangle>
          <v1>38477</v1>
          <v2>38473</v2>
          <v3>38474</v3>
        </triangle>
        <triangle>
          <v1>38477</v1>
          <v2>38476</v2>
          <v3>38473</v3>
        </triangle>
        <triangle>
          <v1>38478</v1>
          <v2>38477</v2>
          <v3>38474</v3>
        </triangle>
        <triangle>
          <v1>38479</v1>
          <v2>38478</v2>
          <v3>38474</v3>
        </triangle>
        <triangle>
          <v1>38479</v1>
          <v2>38474</v2>
          <v3>38475</v3>
        </triangle>
        <triangle>
          <v1>25825</v1>
          <v2>25819</v2>
          <v3>38476</v3>
        </triangle>
        <triangle>
          <v1>38480</v1>
          <v2>38476</v2>
          <v3>38477</v3>
        </triangle>
        <triangle>
          <v1>38480</v1>
          <v2>25825</v2>
          <v3>38476</v3>
        </triangle>
        <triangle>
          <v1>38481</v1>
          <v2>38477</v2>
          <v3>38478</v3>
        </triangle>
        <triangle>
          <v1>38481</v1>
          <v2>38480</v2>
          <v3>38477</v3>
        </triangle>
        <triangle>
          <v1>38482</v1>
          <v2>38481</v2>
          <v3>38478</v3>
        </triangle>
        <triangle>
          <v1>38483</v1>
          <v2>38482</v2>
          <v3>38478</v3>
        </triangle>
        <triangle>
          <v1>38483</v1>
          <v2>38478</v2>
          <v3>38479</v3>
        </triangle>
        <triangle>
          <v1>25831</v1>
          <v2>25825</v2>
          <v3>38480</v3>
        </triangle>
        <triangle>
          <v1>38484</v1>
          <v2>38480</v2>
          <v3>38481</v3>
        </triangle>
        <triangle>
          <v1>38484</v1>
          <v2>25831</v2>
          <v3>38480</v3>
        </triangle>
        <triangle>
          <v1>38485</v1>
          <v2>38481</v2>
          <v3>38482</v3>
        </triangle>
        <triangle>
          <v1>38485</v1>
          <v2>38484</v2>
          <v3>38481</v3>
        </triangle>
        <triangle>
          <v1>38486</v1>
          <v2>38485</v2>
          <v3>38482</v3>
        </triangle>
        <triangle>
          <v1>38487</v1>
          <v2>38486</v2>
          <v3>38482</v3>
        </triangle>
        <triangle>
          <v1>38487</v1>
          <v2>38482</v2>
          <v3>38483</v3>
        </triangle>
        <triangle>
          <v1>25837</v1>
          <v2>25831</v2>
          <v3>38484</v3>
        </triangle>
        <triangle>
          <v1>38488</v1>
          <v2>38484</v2>
          <v3>38485</v3>
        </triangle>
        <triangle>
          <v1>38488</v1>
          <v2>25837</v2>
          <v3>38484</v3>
        </triangle>
        <triangle>
          <v1>38489</v1>
          <v2>38485</v2>
          <v3>38486</v3>
        </triangle>
        <triangle>
          <v1>38489</v1>
          <v2>38488</v2>
          <v3>38485</v3>
        </triangle>
        <triangle>
          <v1>38490</v1>
          <v2>38489</v2>
          <v3>38486</v3>
        </triangle>
        <triangle>
          <v1>38491</v1>
          <v2>38490</v2>
          <v3>38486</v3>
        </triangle>
        <triangle>
          <v1>38491</v1>
          <v2>38486</v2>
          <v3>38487</v3>
        </triangle>
        <triangle>
          <v1>25843</v1>
          <v2>25837</v2>
          <v3>38488</v3>
        </triangle>
        <triangle>
          <v1>38492</v1>
          <v2>38488</v2>
          <v3>38489</v3>
        </triangle>
        <triangle>
          <v1>38492</v1>
          <v2>25843</v2>
          <v3>38488</v3>
        </triangle>
        <triangle>
          <v1>38493</v1>
          <v2>38489</v2>
          <v3>38490</v3>
        </triangle>
        <triangle>
          <v1>38493</v1>
          <v2>38492</v2>
          <v3>38489</v3>
        </triangle>
        <triangle>
          <v1>38494</v1>
          <v2>38493</v2>
          <v3>38490</v3>
        </triangle>
        <triangle>
          <v1>38495</v1>
          <v2>38494</v2>
          <v3>38490</v3>
        </triangle>
        <triangle>
          <v1>38495</v1>
          <v2>38490</v2>
          <v3>38491</v3>
        </triangle>
        <triangle>
          <v1>25849</v1>
          <v2>25843</v2>
          <v3>38492</v3>
        </triangle>
        <triangle>
          <v1>38496</v1>
          <v2>38492</v2>
          <v3>38493</v3>
        </triangle>
        <triangle>
          <v1>38496</v1>
          <v2>25849</v2>
          <v3>38492</v3>
        </triangle>
        <triangle>
          <v1>38497</v1>
          <v2>38493</v2>
          <v3>38494</v3>
        </triangle>
        <triangle>
          <v1>38497</v1>
          <v2>38496</v2>
          <v3>38493</v3>
        </triangle>
        <triangle>
          <v1>38498</v1>
          <v2>38497</v2>
          <v3>38494</v3>
        </triangle>
        <triangle>
          <v1>38499</v1>
          <v2>38498</v2>
          <v3>38494</v3>
        </triangle>
        <triangle>
          <v1>38499</v1>
          <v2>38494</v2>
          <v3>38495</v3>
        </triangle>
        <triangle>
          <v1>25855</v1>
          <v2>25849</v2>
          <v3>38496</v3>
        </triangle>
        <triangle>
          <v1>38500</v1>
          <v2>38496</v2>
          <v3>38497</v3>
        </triangle>
        <triangle>
          <v1>38500</v1>
          <v2>25855</v2>
          <v3>38496</v3>
        </triangle>
        <triangle>
          <v1>38501</v1>
          <v2>38497</v2>
          <v3>38498</v3>
        </triangle>
        <triangle>
          <v1>38501</v1>
          <v2>38500</v2>
          <v3>38497</v3>
        </triangle>
        <triangle>
          <v1>38502</v1>
          <v2>38501</v2>
          <v3>38498</v3>
        </triangle>
        <triangle>
          <v1>38503</v1>
          <v2>38502</v2>
          <v3>38498</v3>
        </triangle>
        <triangle>
          <v1>38503</v1>
          <v2>38498</v2>
          <v3>38499</v3>
        </triangle>
        <triangle>
          <v1>25861</v1>
          <v2>25855</v2>
          <v3>38500</v3>
        </triangle>
        <triangle>
          <v1>38504</v1>
          <v2>38500</v2>
          <v3>38501</v3>
        </triangle>
        <triangle>
          <v1>38504</v1>
          <v2>25861</v2>
          <v3>38500</v3>
        </triangle>
        <triangle>
          <v1>38505</v1>
          <v2>38501</v2>
          <v3>38502</v3>
        </triangle>
        <triangle>
          <v1>38505</v1>
          <v2>38504</v2>
          <v3>38501</v3>
        </triangle>
   