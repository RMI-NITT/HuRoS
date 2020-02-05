function t2 = twodof_manipulator( l,m,r,I,theta,theta_dot,theta_dobdot )
syms c s
c = @(x) cos(x);
s = @(x) sin(x);
M=[
    I(1)+I(2)+m(2)*(l(1)^2)+m(1)*(r(1)^2)+m(2)*(r(2)^2)+2*m(2)*l(1)*r(2)*c(theta(2)) I(2)+m(2)*r(2)^2+m(2)*l(1)*r(2)*c(theta(2));
    I(2)+m(2)*r(2)^2+m(2)*l(1)*r(2)*c(theta(2)) I(2)+m(2)*r(2)^2];
C=[
    -m(2)*l(1)*r(2)*s(theta(2))*(2*theta_dot(1,1)+theta_dot(2,1))*theta_dot(2,1);
    m(2)*l(1)*r(2)*s(theta(2))*theta_dot(1,1)^2];
 
%G=[
 %   m(2)*l(1)*c(theta(1))+r(2)*c(theta(1)+theta(2))*9.81+m(1)*r(1)*c(theta(1))*9.81;
  %  m(2)*r(2)*c(theta(1)+theta(2))];
t1 = (I(1)+I(2)+m(2)*(l(1)^2)+m(1)*(r(1)^2)+m(2)*(r(2)^2)+2*m(2)*l(1)*r(2)*c(theta(2)))*theta_dobdot(1,1)+(I(2)+m(2)*r(2)^2+m(2)*l(1)*r(2)*c(theta(2)))*theta_dobdot(2,1) + ( -m(2)*l(1)*r(2)*s(theta(2))*(2*theta_dot(1,1)+theta_dot(2,1))*theta_dot(2,1) );

t2 =  (I(2)+m(2)*r(2)^2+m(2)*l(1)*r(2)*c(theta(2)))*theta_dobdot(1,1)+(I(2)+m(2)*r(2)^2)*theta_dobdot(2,1) +m(2)*l(1)*r(2)*s(theta(2));


end

