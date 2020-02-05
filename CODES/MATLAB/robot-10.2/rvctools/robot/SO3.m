%SO3 Representation of 3D rotation
%
% This subclasss of RTBPose is an object that represents an SO(3) rotation
%
% Constructor methods::
%  SO3              general constructor
%  SO3.exp          exponentiate an so(3) matrix                         
%  SO3.angvec       rotation about vector
%  SO3.eul          rotation defined by Euler angles
%  SO3.oa           rotation defined by o- and a-vectors
%  SO3.rpy          rotation defined by roll-pitch-yaw angles
%  SO3.Rx           rotation about x-axis
%  SO3.Ry           rotation about y-axis
%  SO3.Rz           rotation about z-axis
%  SO3.rand         random orientation
%  new              new SO3 object
%
% Information and test methods::
%  dim*             returns 3
%  isSE*            returns false
%  issym*           true if rotation matrix has symbolic elements
%
% Display and print methods::
%  plot*            graphically display coordinate frame for pose
%  animate*         graphically animate coordinate frame for pose
%  print*           print the pose in single line format
%  display*         print the pose in human readable matrix form
%  char*            convert to human readable matrix as a string
%
% Operation methods::
%  det              determinant of matrix component
%  eig              eigenvalues of matrix component
%  log              logarithm of rotation matrix
%  inv              inverse
%  simplify*        apply symbolic simplication to all elements
%  interp           interpolate between rotations
%
% Conversion methods::
%  SO3.check        convert object or matrix to SO3 object
%  theta            return rotation angle
%  double           convert to rotation matrix
%  R                convert to rotation matrix
%  SE3              convert to SE3 object with zero translation
%  T                convert to homogeneous transformation matrix with zero translation
%  UnitQuaternion   convert to UnitQuaternion object
%  toangvec         convert to rotation about vector form
%  toeul            convert to Euler angles
%  torpy            convert to roll-pitch-yaw angles
%
% Compatibility methods::
%  isrot*           returns true
%  ishomog*         returns false
%  trprint*         print single line representation
%  trplot*          plot coordinate frame
%  tranimate*       animate coordinate frame
%  tr2eul           convert to Euler angles
%  tr2rpy           convert to roll-pitch-yaw angles
%  trnorm           normalize the rotation matrix
%
% Static methods::
%  check            convert object or matrix to SO2 object
%  exp              exponentiate an so(3) matrix                         
%  isa              check if matrix is 3x3
%  angvec           rotation about vector
%  eul              rotation defined by Euler angles
%  oa               rotation defined by o- and a-vectors
%  rpy              rotation defined by roll-pitch-yaw angles
%  Rx               rotation about x-axis
%  Ry               rotation about y-axis
%  Rz               rotation about z-axis
%
% * means inherited from RTBPose
%
% Operators::
%  +               elementwise addition, result is a matrix
%  -               elementwise subtraction, result is a matrix
%  *               multiplication within group, also group x vector
%  .*              multiplication within group followed by normalization
%  /               multiply by inverse
%  ./              multiply by inverse followed by normalization
%  ==          test equality
%  ~=          test inequality
%
% Properties::
%  n              normal (x) vector
%  o              orientation (y) vector
%  a              approach (z) vector
%
% See also SE2, SO2, SE3, RTBPose.

% Copyright (C) 1993-2017, by Peter I. Corke
%
% This file is part of The Robotics Toolbox for MATLAB (RTB).
% 
% RTB is free software: you can redistribute it and/or modify
% it under the terms of the GNU Lesser General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
% 
% RTB is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Lesser General Public License for more details.
% 
% You should have received a copy of the GNU Leser General Public License
% along with RTB.  If not, see <http://www.gnu.org/licenses/>.
%
% http://www.petercorke.com

classdef SO3 < RTBPose
    
    properties (Dependent=true)
        n
        o
        a
    end
    
    methods
        
        function obj = SO3(a, varargin)
            %SO3.SO3  Construct an SO(2) object
            %
            % P = SO3() is an SO3 object representing null rotation.
            %
            % P = SO3(R) is an SO3 object formed from the rotation 
            % matrix R (3x3)
            %
            % P = SO3(T) is an SO3 object formed from the rotational part 
            % of the homogeneous transformation matrix T (4x4)
            %
            % P = SO3(Q) is an SO3 object that is a copy of the SO3 object Q.            %
            %
            % See also SE3, SO2.
            
            if nargin == 0
                % null rotation
                obj.data = eye(3,3);
            elseif isa(a, 'SO3')
                % copy
                obj.data = a.data;
            elseif SO3.isa(a)
                % from rotation matrix
                for i=1:size(a, 3)
                    obj(i).data = a(:,:,i);
                end
            elseif SE3.isa(a)
                % from homogeneous transformation matrix, rotational part
                for i=1:size(a, 3)
                    obj(i).data = a(1:3,1:3,i);
                end
            end
        end
       
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %  GET AND SET
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        
        function RR = R(obj)
            %SO3.R  Get rotation matrix
            %
            % R = P.R() is the rotation matrix (3x3) associated with the SO3 object P.  If P
            % is a vector (1xN) then R (3x3xN) is a stack of rotation matrices, with
            % the third dimension corresponding to the index of P.
            %
            % See also SO3.T.
            if issym(obj)
                RR = zeros(3,3,length(obj), 'sym');
            else
                RR = zeros(3,3,length(obj));
            end
            for i=1:length(obj)
                RR(:,:,i) = obj(i).data(1:3,1:3);
            end
        end

        function TT = T(obj)
            %SO3.T  Get homogeneous transformation matrix
            %
            % T = P.T() is the homogeneous transformation matrix (4x4) associated with the
            % SO3 object P, and has zero translational component.  If P is a vector
            % (1xN) then T (4x4xN) is a stack of rotation matrices, with the third
            % dimension corresponding to the index of P.
            %
            % See also SO3.T.
            TT = zeros(4,4,length(obj));
            for i=1:length(obj)
                TT(1:3,1:3,i) = obj(i).data(1:3,1:3);
                TT(4,4,i) = 1;
            end
        end
     
        function v = get.n(obj);
            %SO3.n  Get normal vector
            %
            % P.n is the normal vector (3x1), the first column of the rotation matrix,
            % which is the x-axis unit vector.
            %
            % See also SO3.o, SO3.a.
            v = obj.data(1:3,1);
        end
        
        function v = get.o(obj);
            %SO3.o  Get orientation vector
            %
            % P.o is the orientation vector (3x1), the second column of the rotation matrix,
            % which is the y-axis unit vector..
            %
            % See also SO3.n, SO3.a.
            v = obj.data(1:3,2);
        end
        
        function v = get.a(obj);
            %SO3.a  Get approach vector
            %
            % P.a is the approach vector (3x1), the third column of the rotation matrix,
            % which is the z-axis unit vector.
            %
            % See also SO3.n, SO3.o.
            v = obj.data(1:3,3);
        end

        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %  CLASSIC RTB FUNCTION COMPATIBILITY
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

        
        function T = trnorm(obj)
            %SO3.trnorm  Normalize rotation (compatibility)
            %
            % R = P.trnorm() is an SO3 object equivalent to P but
            % normalized (guaranteed to be orthogonal).
            %
            % Notes::
            % - Overrides the classic RTB function trnorm for an SO3 object.
            %
            % See also trnorm.
            for k=1:length(obj)
                T(k) = SE3( trnorm(obj(k).T) );
            end
        end
        
        function rpy = tr2rpy(obj, varargin)
            %SO3.tr2rpy  Convert to RPY angles (compatibility)
            %
            % RPY = P.tr2rpy(OPTIONS) is a vector (1x3) of roll-pitch-yaw angles
            % equivalent to the rotation P (SO3 object).
            %
            % Notes::
            % - Overrides the classic RTB function tr2rpy for an SO3 object.
            % - All the options of tr2rpy apply.
            % - Defaults to ZYX order.
            %
            % See also tr2rpy.
            rpy = tr2rpy(obj.R, varargin{:});
        end
        
        function eul = tr2eul(obj, varargin)
            %SO3.tr2eul  Convert to Euler angles (compatibility)
            %
            % RPY = P.tr2eul(OPTIONS) is a vector (1x3) of ZYZ Euler angles
            % equivalent to the rotation P (SO3 object).
            %
            % Notes::
            % - Overrides the classic RTB function tr2eul for an SO3 object.
            % - All the options of tr2eul apply.
            %
            % See also tr2eul.
            eul = tr2eul(obj.R, varargin{:});
        end
        
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %  COMPOSITION
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
       
        function out = times(obj, a)
            %SO3.times  Compound SO3 objects and normalize
            %
            % R = P.*Q is an SO3 object representing the composition of the two
            % rotations described by the SO3 objects P and Q, which is matrix multiplication
            % of their orthonormal rotation matrices followed by normalization.
            %
            % If either, or both, of P or Q are vectors, then the result is a vector.
            %
            % If P is a vector (1xN) then R is a vector (1xN) such that R(i) = P(i).*Q.
            %
            % If Q is a vector (1xN) then R is a vector (1xN) such thatR(i) = P.*Q(i).
            %
            % If both P and Q are vectors (1xN) then R is a vector (1xN) such that 
            % R(i) = P(i).*R(i).
            %
            % See also RTBPose.mtimes, SO3.divide, trnorm.
            
            % do the multiplication
            out = mtimes(obj, a);
            
            % now normalize
            if isa(out, 'SO3')
                % created an array of SE3's
                for i=1:length(out)
                    out(i).data = trnorm(out(i).data);
                end
            end
        end
        

        function out = rdivide(obj, a)
            %SO3.mrdivide  Compound SO3 object with inverse and normalize
            %
            % P./Q is the composition, or matrix multiplication of SO3 object P by the
            % inverse of SO3 object Q. If either of P or Q are vectors, then the result
            % is a vector where each element is the product of the object scalar and
            % the corresponding element in the object vector.  If both P and Q are
            % vectors they must be of the same length, and the result is the
            % elementwise product of the two vectors.
            %
            % See also SO3.mrdivide, SO3.times, trnorm.
            
            % do the division
            out = mrdivide(obj, a);
            
            % now normalize
            % created an array of SO3's
            for i=1:length(out)
                out(i).data = trnorm(out(i).data);
            end
        end
        
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%  OPERATIONS
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        
        function ir = inv(obj)
            %SO3.inv  Inverse of SO3 object
            %
            % Q = inv(P) is the inverse of the SO3 object P.  P*Q will be the identity
            % matrix.
            %
            % Notes::
            % - This is simply the transpose of the matrix.
            ir = SO3(obj.R');
        end

        function d = det(obj)
            %SO3.inv  Determinant of SO3 object
            %
            % det(P) is the determinant of the SO3 object P and should always be +1.
            d = det(obj.R);
        end
        
        function R = interp(obj1, obj2, s)
            %SO3.interp Interpolate between SO3 objects
            %
            % P1.interp(P2, s) is an SO3 object representing a slerp interpolation
            % between rotations represented by SO3 objects P1 and P2.  s varies from 0
            % (P1) to 1 (P2).  If s is a vector (1xN) then the result will be a vector
            % of SO3 objects.
            %
            % P1.interp(P2,N) as above but returns a vector (1xN) of SO3 objects
            % interpolated between P1 and P2 in N steps.
            %
            % Notes::
            % - It is an error if S is outside the interval 0 to 1.
            %
            % See also UnitQuaternion.
            
            if (length(s) == 1) && (floor(s) == s) && (s > 1)
                % is an integer, interpolate a sequence this long
                s = linspace(0, 1, s);
            else
                assert(all(s>=0 & s<=1), 'RTB:SO3:interp:badarg', 's must be in the interval [0,1]');
            end
            R = SO3( obj1.UnitQuaternion.interp(obj2.UnitQuaternion, s) );
        end

        function varargout = eig(obj, varargin)
            %SO3.eig  Eigenvalues and eigenvectors
            %
            % E = eig(P) is a column vector containing the eigenvalues of the the
            % rotation matrix of the SO3 object P.
            %
            % [V,D] = eig(P) produces a diagonal matrix D of eigenvalues and 
            % a full matrix V whose columns are the corresponding eigenvectors  
            % so that A*V = V*D.
            %
            % See also eig.
            [varargout{1:nargout}] = eig(obj.data, varargin{:});
        end
        
        function S = log(obj)
            %SE2.log  Lie algebra
            %
            % se2 = P.log() is the Lie algebra augmented skew-symmetric matrix (3x3)
            % corresponding to the SE2 object P.
            %
            % See also SE2.Twist, trlog.
            
            S = trlog(obj.data);
        end
        

        
        %         function o = set.R(obj, data)
        %             obj.data(1:3,1:3) = data;
        %             o = obj;
        %         end
        
        %         function TT = T(obj)
        %             TT = eye(4,4);
        %             TT(1:3,1:3) = obj.data(1:3,1:3);
        %          end
        
        % TODO singularity for XYZ case,
        
        function rpy = torpy(obj, varargin)
            %SO3.RPY Convert to roll-pitch-yaw angles
            %
            % RPY = P.torpy(options) are the roll-pitch-yaw angles (1x3) corresponding
            % to the rotational part of the SO3 object P. The 3 angles RPY=[R,P,Y]
            % correspond to sequential rotations about the Z, Y and X axes
            % respectively.
            %
            % If P is a vector (1xN) then each row of RPY corresponds to an element of
            % the vector.
            %
            % Options::
            %  'deg'   Compute angles in degrees (radians default)
            %  'xyz'   Return solution for sequential rotations about X, Y, Z axes
            %  'yxz'   Return solution for sequential rotations about Y, X, Z axes
            %
            % Notes::
            % - There is a singula                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        