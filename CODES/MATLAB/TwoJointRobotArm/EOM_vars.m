clear
clc

syms m1 m2;
syms H1 H2;
syms u1 u2;
syms R1 R2;
syms J0 J1 J2;
syms K11 K12;
syms K21 K22;
syms r alpha;
syms th1 th2 th1dot th2dot;
syms TL;
syms g;

% m1=2;
% m2=2;
% H1=0.4;
% H2=0.4;
% g=10;
% R1=1;
% R2=1;
% K11=0.5;
% K12=1;
% K21=0.5;
% K22=1;
%J0 = (m1*H1*H1)/3;
%J2 = (m2*H2*H2)/3;

%r = sqrt(H1^2 + (H2/2)^2 - H1*H2*cos(pi - th2));

%alpha = asin(((H2/2)*sin(pi - th2))/r) + th1;

%J1 = J0 + ((r^2)*m2);

th1dotdot = (K21*u1)/(R1*J1) - (K11*K21*th1dot)/(R1*J1) - (r*m2*g*cos(alpha))/J1 - (H1*m1*g*cos(th1))/(2*J1);
th2dotdot = (K22*u2)/(R2*J2) - (K12*K22*th2dot)/(R2*J2) - (H2*m2*g*cos(th1+th2))/(2*J2) - TL/J2;
th2dotdot = th2dotdot - J2*th1dotdot;

 