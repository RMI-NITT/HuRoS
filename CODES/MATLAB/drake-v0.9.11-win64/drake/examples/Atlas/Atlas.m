classdef Atlas < TimeSteppingRigidBodyManipulator & Biped
  methods

    function obj=Atlas(urdf,options)
      typecheck(urdf,'char');

      if nargin < 2
        options = struct();
      end
      if ~isfield(options,'dt')
        options.dt = 0.001;
      end
      if ~isfield(options,'floating')
        options.floating = true;
      end
      if ~isfield(options,'terrain')
        options.terrain = RigidBodyFlatTerrain;
      end
      if ~isfield(options,'hand_right')
        options.hand_right = 'none';
      end
      if ~isfield(options,'hand_left')
        options.hand_left = 'none';
      end
      if ~isfield(options,'external_force')
        options.external_force = [];
      end
      if ~isfield(options,'atlas_version') 
        options.atlas_version = 5; 
      end

      w = warning('off','Drake:RigidBodyManipulator:UnsupportedVelocityLimits');

      obj = obj@TimeSteppingRigidBodyManipulator(urdf,options.dt,options);
      obj = obj@Biped('r_foot_sole', 'l_foot_sole');
      warning(w);

      if options.atlas_version == 3
        hand_position_right = [0; -0.195; -0.01];
        hand_position_left = [0; 0.195; 0.01];
        hand_orientation_right = [0; -pi/2; pi];
        hand_orientation_left = [0; -pi/2; 0];
      elseif options.atlas_version == 4
        hand_position_right = [0; -0.195; -0.01];
        hand_position_left = hand_position_right;
        hand_orientation_right = [0; -pi/2; pi];
        hand_orientation_left = [0; pi/2; pi];
      elseif options.atlas_version == 5
        hand_position_right = [0; -0.195; 0.0];
        hand_orientation_right = [0; -pi/2; pi];
        hand_position_left = [0; -0.195; 0.0];
        hand_orientation_left = [0; -pi/2; pi];
      end

      hand=options.hand_right;
      if (~strcmp(hand, 'none'))
        clear options_hand;
        if (strcmp(hand, 'robotiq') || strcmp(hand, 'robotiq_tendons') || strcmp(hand, 'robotiq_simple'))
          options_hand.weld_to_link = findLinkId(obj,'r_hand');
          obj.hand_right = 1;
          obj.hand_right_kind = hand;
          obj = obj.addRobotFromURDF(getFullPathFromRelativePath(['urdf/', hand, '.urdf']), hand_position_right, hand_orientation_right, options_hand);
        elseif (strcmp(hand, 'robotiq_weight_only'))
          % Adds a box with weight roughly approximating the hands, so that
          % the controllers know what's up
          options_hand.weld_to_link = findLinkId(obj,'r_hand');
          obj = obj.addRobotFromURDF(getFullPathFromRelativePath('urdf/robotiq_box.urdf'), hand_position_right, hand_orientation_right, options_hand);
        else
          error('unsupported hand type');
        end
      end
      hand=options.hand_left;
      if (~strcmp(hand, 'none'))
        clear options_hand;
        if (strcmp(hand, 'robotiq') || strcmp(hand, 'robotiq_tendons') || strcmp(hand, 'robotiq_simple'))
          options_hand.weld_to_link = findLinkId(obj,'l_hand');
          obj.hand_left = 1;
          obj.hand_left_kind = hand;
          obj = obj.addRobotFromURDF(getFullPathFromRelativePath(['urdf/', hand, '.urdf']), hand_position_left,hand_orientation_left, options_hand);
        elseif (strcmp(hand, 'robotiq_weight_only'))
          % Adds a box with weight roughly approximating the hands, so that
          % the controllers know what's up
          options_hand.weld_to_link = findLinkId(obj,'l_hand');
          obj = obj.addRobotFromURDF(getFullPathFromRelativePath('urdf/robotiq_box.urdf'), hand_position_left,hand_orientation_left, options_hand);
        else
          error('unsupported hand type');
        end
      end

      % Add a force on a specified link if we want!
      if ~isempty(options.external_force)
        % For compile purposes, record that we have external force applied to a link
        % (this affects input frames)
        obj.external_force = findLinkId(obj,options.external_force);
        options_ef.weld_to_link = obj.external_force;
        obj = obj.addRobotFromURDF(fullfile(getDrakePath,'util','three_dof_force.urdf'), ...
          [0; 0; 0], [0; 0; 0], options_ef);
      end

      if options.floating
        % could also do fixed point search here
        obj = obj.setInitialState(obj.resolveConstraints(zeros(obj.getNumStates(),1)));
      else
        % TEMP HACK to get by resolveConstraints
        for i=1:length(obj.manip.body), obj.manip.body(i).contact_pts=[]; end
        obj.manip = compile(obj.manip);
        obj = obj.setInitialState(zeros(obj.getNumStates(),1));
      end
      if isfield(options, 'atlas_version')
        obj.atlas_version = options.atlas_version;
      end

      lastwarn = warning('off', 'Drake:RigidBodySupportState:NoSupportSurface');
      obj.left_full_support = RigidBodySupportState(obj,obj.foot_body_id.left);
      obj.left_toe_support = RigidBodySupportState(obj,obj.foot_body_id.left,struct('contact_groups',{{'toe'}}));
      obj.right_full_support = RigidBodySupportState(obj,obj.foot_body_id.right);
      obj.right_toe_support = RigidBodySupportState(obj,obj.foot_body_id.right,struct('contact_groups',{{'toe'}}));
      obj.left_full_right_full_support = RigidBodySupportState(obj,[obj.foot_body_id.left,obj.foot_body_id.right]);
      obj.left_toe_right_full_support = RigidBodySupportState(obj,[obj.foot_body_id.left,obj.foot_body_id.right],struct('contact_groups',{{{'toe'},{'heel','toe'}}}));
      obj.left_full_right_toe_support = RigidBodySupportState(obj,[obj.foot_body_id.left,obj.foot_body_id.right],struct('contact_groups',{{{'heel','toe'},{'toe'}}}));
      warning(lastwarn);
    end

    function obj = compile(obj)
      obj = compile@TimeSteppingRigidBodyManipulator(obj);

      % Construct state vector itself -- start by replacing the
      % atlasPosition and atlasVelocity frames with a single
      % larger state frame
      if (strcmp(obj.manip.getStateFrame().getFrameByNum(1).name, 'atlasPosition'))
        atlas_state_frame = atlasFrames.AtlasState(obj);
      else
        atlas_state_frame = obj.manip.getStateFrame();
        atlas_state_frame = replaceFrameNum(atlas_state_frame,1,atlasFrames.AtlasState(obj));
      end

      % Sub in handstates for the hands
      % If we sub in the order that they are added
      % we should get this in the right order
      if (obj.hand_right > 0)
        id = atlas_state_frame.getFrameNumByName('s-model_articulatedPosition+s-model_articulatedVelocity');
        if (length(id) > 1)
          id = id(1);
        end
        atlas_state_frame = replaceFrameNum(atlas_state_frame,id,atlasFrames.HandState(obj,id,'right_atlasFrames.HandState'));
      end
      if (obj.hand_left > 0)
        id = atlas_state_frame.getFrameNumByName('s-model_articulatedPosition+s-model_articulatedVelocity');
        if (length(id) > 1)
          id = id(1);
        end
        atlas_state_frame = replaceFrameNum(atlas_state_frame,id,atlasFrames.HandState(obj,id,'left_atlasFrames.HandState'));
      end

      tsmanip_state_frame = obj.getStateFrame();
      if tsmanip_state_frame.dim>atlas_state_frame.dim
        tsmanip_state_frame.frame{1} = atlas_state_frame;
        state_frame = tsmanip_state_frame;
      else
        state_frame = atlas_state_frame;
      end
      obj.manip = obj.manip.setStateFrame(atlas_state_frame);
      obj = obj.setStateFrame(state_frame);

      % Same bit of complexity for input frame to get hand inputs
      if (obj.hand_right > 0 || obj.hand_left > 0 || obj.external_force > 0)
        input_frame = getInputFrame(obj);
        input_frame  = replaceFrameNum(input_frame,1,atlasFrames.AtlasInput(obj));
      else
        input_frame = atlasFrames.AtlasInput(obj);
      end
      if (obj.hand_right > 0)
        id = input_frame.getFrameNumByName('s-model_articulatedInput');
        if (length(id) > 1)
          id = id(1);
        end
        input_frame = replaceFrameNum(input_frame,id,atlasFrames.HandInput(obj,id,'right_atlasFrames.HandInput'));
      end
      if (obj.hand_left > 0)
        id = input_frame.getFrameNumByName('s-model_articulatedInput');
        if (length(id) > 1)
          id = id(1);
        end
        input_frame = replaceFrameNum(input_frame,id,atlasFrames.HandInput(ob                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         