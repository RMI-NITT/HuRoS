�PNG

   IHDR   8   8   ��;  YIDATh��Z=hA�*�զ��O�؈xB:SY\!V�`��!��"A�F�#��X��"A�IH�x	�^�����Hc4FT���6̾�ٝ�ݽlp?xW�����y����b"���q�PcH�!I�࿚�F*nHʐC�%m��o؉��\���ǈ��\�bT0V",�R<%w6�>9x�]�͇�#}���ƧVuV����
��BGʞo�4<\\�e���# �
��s�O�uE�/�Xw��=gQ�������O�+0��LT3Z�(���?�_�X���D�r�����Y���jXOc��='�'I���",�|z��U8l���u�
T�9�[�tϵ�D�֫�E��`;	�/J?��'���j���:��]�� ��*�o˿�M��r��+}drѶ=��@R^�%�
hԓ<\m�����*��M�����ˡ0Oz�u�����Y� C	B�_���i4̊N�:�J�i��\õ!��v�.i.���8��1�6'��k˥/�v��v_��,�������I[���,�O�)�jLXb��%��h�@�ٚ:rH&h'rXe;rN��Z��%�B$��9uM���t � J�h7� ���d������Qru��k�%�V@�j��XYt��ب��L����K�V���l� ��;[�s+���}3H��2Ä�>t(��*҂J�""(c5��W���'c�K�鷽*'sJ�~�g�~hg��o���N��?SrQs/�lb,���UjK��*س� ��\�"��,���d!x��ԭZ@�0/리E"� 8B��^��l��S<w�%�K֨P��B��c��uE��)]�n���%`ivq�0��w:�[�Y��5������Hd�oӊ,��;8X�o��˸c����;X���S�WI,�;Ƀ��SD����EP�<n��
�;���M�8H>ꝟ�kR���<�
�L�9�.�� ��t%L���m��B��F 	�+!��;������(�=��8�J]��}f�k���:�� w),z�����A��d����Z�_Xʛ�B1�3�ԡ���DТ�\��yw�!����1�����;?������o���,�۶"D�a��~#`�T�    IEND�B`�                                                                                                                                                                                                                                                                                                                                                                                    obj.body = obj.robot.parseBodyOrFrameID(body);
      obj.body_name = obj.robot.getBodyOrFrameName(obj.body);
      obj.type = RigidBodyConstraint.WorldPositionConstraintType;
      if robot.getMexModelPtr~=0 && exist('constructPtrRigidBodyConstraintmex','file')
        obj.mex_ptr = constructPtrRigidBodyConstraintmex(RigidBodyConstraint.WorldPositionConstraintType,robot.getMexModelPtr,body,pts,lb,ub,tspan);
      end
    end
    
    function joint_idx = kinematicsPathJoints(obj)
      [~,joint_path] = obj.robot.findKinematicPath(1,obj.body);
      if isa(obj.robot,'TimeSteppingRigidBodyManipulator')
        joint_idx = vertcat(obj.robot.getManipulator().body(joint_path).position_num)';
      else
        joint_idx = vertcat(obj.robot.body(joint_path).position_num)';
      end
    end
    
  end
end
