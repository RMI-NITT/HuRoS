%{ 
    Date : 18.12.2018
    Author : Tuna Orhanli
    Program : 2 DOF Planar Manipulator Dynamic Equations Solver (Continuous Tau)
    Additional Notes : 
    1- Uses Numerical Integration  
    2- Takes Tau's as cos(t) and sin(t) (given arbitrarily)
%}
    clc;clear all;close all;
    % Define constants
    
    m1 = 50; m2 = 50; % kg
    l1 = 1; l2 = 1;   % meter
    b1 = 0.5; b2 = 0.5; % COM distance (meter)
    I1 = (1/3)*(m1)*(l1^2); % Inertia
    I2 = (1/3)*(m2)*(l2^2); % Inertia
    g = -9.8; % Gravity Acceleration
    t0 = 0 ; tf = 5;   % Integration Duration
    tspan = [t0 tf];
    
    % Define initial values theta1, theta1_dot, theta2, theta2_dot
    
    theta1 = deg2rad(0); theta2 = deg2rad(0);  % Initial Positions (Angle)
    theta1_dot = deg2rad(0); theta2_dot = deg2rad(0); % Initial Angular Velocities
    IC = [theta1,theta1_dot,theta2,theta2_dot]; % Initial conditions
    
    % Define time dependent parameters (torques)
    
    tau1 = @(time) cos(time);  %  tau1 cos sinyali olsun
    tau2 = @(time) sin(time);  %  tau2 sin sinyali olsun
    % Define inertia matrix parameters
    
    d11 = @(S) (m1*b1^2)+m2*(l1^2+b2^2+2*l1*b2*cos(S(3)))+I1+I2;
    d12 = @(S) m2*(b2^2+l1*b2*cos(S(3)))+I2;
    d21 = @(S) m2*(b2^2+l1*b2*cos(S(3)))+I2;
    d22 = m2*b2^2+I2;
    
    % Define Christoffel Symbols
    
    c121 = @(S) -1*l1*b2*m2*sin(S(3));
    c211 = @(S) -1*l1*b2*m2*sin(S(3));
    c221 = @(S) -1*l1*b2*m2*sin(S(3));
    c112 = @(S) l1*b2*m2*sin(S(3));
    
    % Define gravity terms
    
    g1 = @(S) (m1*b1+m2*l1)*g*cos(S(1)) + m2*b2*g*cos(S(1)+S(3));
    g2 = @(S)  m2*b2*g*cos(S(1)+S(3));
    
     % Define necessary coefficients
    
     
    theta1_katsayi = @(S) (d12(S)*d22)/(d12(S)*d21(S)-d11(S)*d22);
    theta2_katsayi = @(S) (d11(S)*d21(S))/(d11(S)*d22-d12(S)*d21(S));
    
    % Define terms of theta1
    
    theta1_1 =  @(S) ((-c112(S)/d22)*(S(2)^2));
    theta1_2 =  @(S) ((c221(S)/d12(S))*(S(4)^2));
    theta1_3 =  @(S) (((c121(S)+c211(S))/d12(S))*S(2)*S(4));
    theta1_4 =  @(S) (g1(S)/d12(S));
    theta1_5 =  @(S) (-g2(S)/d22);
    theta1_6 =  @(time,S) (-tau1(time)/d12(S));
    theta1_7 =  @(time) (tau2(time)/d22);
    
    % Define terms of theta2
    
    theta2_1 =  @(S) ((-c112(S)/d21(S))*(S(2)^2));
    theta2_2 =  @(S) ((c221(S)/d11(S))*(S(4)^2));
    theta2_3 =  @(S) (((c121(S)+c211(S))/d11(S))*S(2)*S(4));
    theta2_4 =  @(S) (g1(S)/d11(S));
    theta2_5 =  @(S) (-g2(S)/d21(S));
    theta2_6 =  @(time,S) (-tau1(time)/d11(S));
    theta2_7 =  @(time,S) (tau2(time)/d21(S));
    
    % Define system of first order differential equations
    
    SDOT = @(time,S) ...
        [S(2);
        theta1_katsayi(S)*(theta1_1(S)+theta1_2(S)+theta1_3(S)+theta1_4(S)+theta1_5(S)+theta1_6(time,S)+theta1_7(time));
        S(4);
        theta2_katsayi(S)*(theta2_1(S)+theta2_2(S)+theta2_3(S)+theta2_4(S)+theta2_5(S)+theta2_6(time,S)+theta2_7(time,S))];
    
    % Numerical Integration by means of 'ODE45' Runge Kutta Method
    
    [time state_values] = ode45 (SDOT, tspan, IC);
    theta1 = state_values(:,1);
    theta1_dot = state_values(:,2);
    theta2 = state_values(:,3);
    theta2_dot = state_values(:,4);
    
    subplot(2,2,1)
    plot(time,rad2deg(theta1)),xlabel('time (s)'),ylabel('Joint Angle 1')   
    title(' Joint Angle 1 (Deg) vs Time')
    
    subplot(2,2,2)
    plot(time,rad2deg(theta1_dot)),xlabel('time (s)'),ylabel('Joint Velocity 1')   
    title(' Joint Velocity 1 vs Time')
    
    subplot(2,2,3)
    plot(time,rad2deg(theta2)),xlabel('time (s)'),ylabel('Joint Angle 2')   
    title(' Joint Angle 2 (Deg) vs Time')
    
    subplot(2,2,4)
    plot(time,rad2deg(theta2_dot)),xlabel('time (s)'),ylabel('Joint Velocity 2')   
    title(' Joint Velocity 2 vs Time')
