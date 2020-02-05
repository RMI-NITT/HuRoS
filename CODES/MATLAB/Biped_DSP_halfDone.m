%parameters
l = [ 0.35; 0.25]  %metre
lc = [0.175; 0.125]

m = [1; 1; 0.3]
total_mass = 2*(m(1)+m(2))+m(3)

theta = [ pi/3; pi/2; -5*pi/6; -pi/6; pi/2; -2*pi/3]

alpha = [ pi/2; -pi/2; -pi/2; pi/2]

b = 0.20
bc = 0

foot_length = 0.1
foot_width = 0.06

x0 = 0;
y0 = 0;
z0 = 0;
point0 = [x0,y0,z0]

x1 = l(2)*sin(alpha(1))*cos(theta(1))
y1 = l(2)*sin(theta(1))*cos(alpha(1))
z1 = l(2)*sin(theta(1))*sin(alpha(1))
point1 = [x1,y1,z1]

link1 = [point0;point1]

dx2 = l(1)*sin(alpha(1))*cos(theta(1)+theta(2))
dy2 = l(1)*sin(theta(1)+theta(2))*cos(alpha(1))
dz2 = l(1)*sin(theta(1)+theta(2))*sin(alpha(1))

x2 = x1 + dx2
y2 = y1 + dy2
z2 = z1 + dz2
point2 = [x2,y2,z2]

link2 = [point1;point2];

dx3 = 0
dy3 = b*cos(alpha(1)+alpha(2))
dz3 = b*sin(alpha(1)+alpha(2))

x3 = x2 + dx3
y3 = y2 + dy3
z3 = z2 + dz3
point3 = [x3,y3,z3]

link3 = [point2;point3];

dx4 = l(1)*sin(alpha(4))*cos(theta(5)+theta(6))
dy4 = l(1)*sin(theta(5)+theta(6))*cos(alpha(4))
dz4 = l(1)*sin(theta(5)+theta(6))*sin(alpha(4))

x4 = x3 + dx4
y4 = y3 + dy4
z4 = z3 + dz4
point4 = [x4,y4,z4]

link4 = [point3;point4];

dx5 = l(2)*sin(alpha(4))*cos(theta(6))
dy5 = l(2)*sin(theta(6))*cos(alpha(4))
dz5 = l(2)*sin(theta(6))*sin(alpha(4))

x5 = x4 + dx5
y5 = y4 + dy5
z5 = z4 + dz5
point5 = [x5,y5,z5]

link5 = [point4;point5]

%left_foot1 = line([x0-foot_length; x0+foot_length],[y0;y0],[z0;z0])
%left_foot2 = line([x0;x0],[y0-foot_width;y0+foot_width],[z0;z0])
%right_foot2 = line([x5;x5],[y5-foot_width;y5+foot_width],[z5;z5])
%right_foot1 = line([x5-foot_length; x0+foot_length],[y5;y5],[z5;z5])

xcom = (m(2)*(x1*lc(2)/l(2))+m(1)*(x1+dx2*lc(1)/l(1))+m(3)*(x2+dx3*bc/b)+m(1)*(x3+dx4*lc(1)/l(1))+m(2)*(x4+dx5*lc(2)/l(2)))
xcom = xcom/total_mass

ycom = (m(2)*(y1*lc(2)/l(2))+m(1)*(y1+dy2*lc(1)/l(1))+m(3)*(y2+dy3*(b/2)/b)+m(1)*(y3+dy4*lc(1)/l(1))+m(2)*(y4+dy5*lc(2)/l(2)))
ycom = ycom/total_mass

zcom = (m(2)*(z1*lc(2)/l(2))+m(1)*(z1+dz2*lc(1)/l(1))+m(3)*(z2+dz3*(b/2)/b)+m(1)*(z3+dz4*lc(1)/l(1))+m(2)*(z4+dz5*lc(2)/l(2)))
zcom = zcom/total_mass

%single stance leg swing IK with body frame at the torso/hip

body_frame_x = -0.1781
body_frame_y = 0.1
body_frame_z  = 0.3915
body_frame = [body_frame_x, body_frame_y, body_frame_z]

% left, right foot end effectors
%give function of foot trajectories for both the feet here


%right foot forward
for i=1:27
    ssp_ik_x0(i) = i*0.01;
    ssp_ik_y0(i) = 0;
    ssp_ik_z0(i) = 0;
    
    ssp_ik_x5(i) = 0;
    ssp_ik_y5(i) = 0.2;
    ssp_ik_z5(i) = 0;
end 


%right foot backward
for i=28:55
    ssp_ik_x0(i) = (55-i)*0.01;
    ssp_ik_y0(i) = 0;
    ssp_ik_z0(i) = 0;
    
    ssp_ik_x5(i) = 0;
    ssp_ik_y5(i) = 0.2;
    ssp_ik_z5(i) = 0;
end


%{
%left foot forward
for i=56:81
    ssp_ik_x0(i) = 0;
    ssp_ik_y0(i) = 0;
    ssp_ik_z0(i) = 0;
    
    ssp_ik_x5(i) = (i-56)*0.01;
    ssp_ik_y5(i) = 0.2;
    ssp_ik_z5(i) = 0;
end

%left foot backward
for i=82:108
    ssp_ik_x0(i) = 0;
    ssp_ik_y0(i) = 0;
    ssp_ik_z0(i) = 0;
    
    ssp_ik_x5(i) = (108-i)*0.01;
    ssp_ik_y5(i) = 0.2;
    ssp_ik_z5(i) = 0;
end
%}


%solving IK for single stance phase 
j = i
for i=1:j
    ssp_ik_theta = [0 0 0 0 0 0]
    theta = [0 0 0 0 0 0 ]
        
    ssp_ik_x2(i) = body_frame_x; 
    ssp_ik_y2(i) = body_frame_y - (b/2);
    ssp_ik_z2(i) = body_frame_z;
    
    ssp_ik_x3(i) = body_frame_x ;
    ssp_ik_y3(i) = body_frame_y + (b/2);
    ssp_ik_z3(i) = body_frame_z;

    left_dist = sqrt((ssp_ik_x3(i) - ssp_ik_x5(i))^2 + (ssp_ik_y3(i) - ssp_ik_y5(i))^2 + (ssp_ik_z3(i) - ssp_ik_z5(i))^2);
    right_dist = sqrt((ssp_ik_x2(i) - ssp_ik_x0(i))^2 + (ssp_ik_y2(i) - ssp_ik_y0(i))^2 + (ssp_ik_z2(i) - ssp_ik_z0(i))^2);

    left_phi = acos(((left_dist^2 + (ssp_ik_z3(i) - ssp_ik_z5(i))^2) - (ssp_ik_x3(i) - ssp_ik_x5(i))^2)/(2*left_dist*(ssp_ik_z3(i) - ssp_ik_z5(i))));
    left_slope(i) = (pi/2) + left_phi;
    right_phi = acos(((right_dist^2 + (ssp_ik_z2(i) - ssp_ik_z0(i))^2) - (ssp_ik_x2(i) - ssp_ik_x0(i))^2)/(2*right_dist*(ssp_ik_z2(i) - ssp_ik_z0(i))));
    right_slope(i) = (pi/2) + right_phi;

    right_leg_angle1 = acos((right_dist^2 + l(1)^2 - l(2)^2)/(2*right_dist*l(1))); 
    right_leg_angle2 = acos((-right_dist^2 + l(1)^2 + l(2)^2)/(2*l(2)*l(1))) ;
    right_leg_angle3 = acos((right_dist^2 - l(1)^2 + l(2)^2)/(2*right_dist*l(2))); 

    ssp_ik_theta(1) = right_slope(i) - right_leg_angle3;
    ssp_ik_theta(2) = pi - right_leg_angle2;
    ssp_ik_theta(3) = ssp_ik_theta(1) + ssp_ik_theta(2);

    left_leg_angle1 = acos((left_dist^2 + l(1)^2 - l(2)^2)/(2*left_dist*l(1))) ;
    left_leg_angle2 = acos((-left_dist^2 + l(1)^2 + l(2)^2)/(2*l(2)*l(1))) ;
    left_leg_angle3 = acos((left_dist^2 - l(1)^2 + l(2)^2)/(2*left_dist*l(2))) ;

    ssp_ik_theta(5) = (pi - left_leg_angle2);
    ssp_ik_theta(6) = -(pi - left_slope(i) + left_leg_angle3);   %doubtu doubtu doubtu
    ssp_ik_theta(4) = ssp_ik_theta(5) + ssp_ik_theta(6);

    theta = ssp_ik_theta
    
    dx1 = l(2)*sin(alpha(1))*cos(theta(1));
    dy1 = l(2)*sin(theta(1))*cos(alpha(1));
    dz1 = l(2)*sin(theta(1))*sin(alpha(1)) ;

    ssp_ik_x1(i) = ssp_ik_x0(i) + dx1;
    ssp_ik_y1(i) = ssp_ik_y0(i) + dy1;
    ssp_ik_z1(i) = ssp_ik_z0(i) + dz1;
    
    dx2 = l(1)*sin(alpha(1))*cos(theta(1)+theta(2));
    dy2 = l(1)*sin(theta(1)+theta(2))*cos(alpha(1));
    dz2 = l(1)*sin(theta(1)+theta(2))*sin(alpha(1));

    ssp_ik_x2(i) = ssp_ik_x1(i) + dx2;
    ssp_ik_y2(i) = ssp_ik_y1(i) + dy2;
    ssp_ik_z2(i) = ssp_ik_z1(i) + dz2;
        
    dx4 = l(1)*sin(alpha(4))*cos(theta(5)+theta(6));
    dy4 = l(1)*sin(theta(5)+theta(6))*cos(alpha(4));
    dz4 = l(1)*sin(theta(5)+theta(6))*sin(alpha(4));

    ssp_ik_x4(i) = ssp_ik_x3(i) + dx4;
    ssp_ik_y4(i) = ssp_ik_y3(i) + dy4;
    ssp_ik_z4(i) = ssp_ik_z3(i) + dz4;
    
    xcom(i) = (m(2)*(ssp_ik_x1(i)*lc(2)/l(2))+m(1)*(ssp_ik_x1(i)+dx2*lc(1)/l(1))+m(3)*(ssp_ik_x2(i)+dx3*bc/b)+m(1)*(ssp_ik_x3(i)+dx4*lc(1)/l(1))+m(2)*(ssp_ik_x4(i)+dx5*lc(2)/l(2)))
    xcom(i) = xcom(i)/total_mass

    ycom(i) = (m(2)*(ssp_ik_y1(i)*lc(2)/l(2))+m(1)*(ssp_ik_y1(i)+dy2*lc(1)/l(1))+m(3)*(ssp_ik_y2(i)+dy3*(b/2)/b)+m(1)*(ssp_ik_y3(i)+dy4*lc(1)/l(1))+m(2)*(ssp_ik_y4(i)+dy5*lc(2)/l(2)))
    ycom(i) = ycom(i)/total_mass

    zcom(i) = (m(2)*(ssp_ik_z1(i)*lc(2)/l(2))+m(1)*(ssp_ik_z1(i)+dz2*lc(1)/l(1))+m(3)*(ssp_ik_z2(i)+dz3*(b/2)/b)+m(1)*(ssp_ik_z3(i)+dz4*lc(1)/l(1))+m(2)*(ssp_ik_z4(i)+dz5*lc(2)/l(2)))
    zcom(i) = zcom(i)/total_mass
end
for k=1:1
    for i=1:j   
         clf('reset')
    
        rectangle('Position',[-1 -1 2 2], 'FaceColor',[0 1 1])
    
        axis([-1 1.5 -1 1 -0.1 0.6])
        hold on
    
        ssp_ik_point0 = [ssp_ik_x0(i) - 0.2,ssp_ik_y0(i),ssp_ik_z0(i)];
        ssp_ik_point1 = [ssp_ik_x1(i) - 0.2, ssp_ik_y1(i), ssp_ik_z1(i)];
        ssp_ik_point2 = [ssp_ik_x2(i) - 0.2,ssp_ik_y2(i),ssp_ik_z2(i)];
        ssp_ik_point3 = [ssp_ik_x3(i) - 0.2,ssp_ik_y3(i),ssp_ik_z3(i)];
        ssp_ik_point4 = [ssp_ik_x4(i) - 0.2, ssp_ik_y4(i), ssp_ik_z4(i)];
        ssp_ik_point5 = [ssp_ik_x5(i) - 0.2,ssp_ik_y5(i),ssp_ik_z5(i)];
    
        ssp_ik_link1 = [ssp_ik_point0;ssp_ik_point1];
        plot3(ssp_ik_link1(:,1),ssp_ik_link1(:,2),ssp_ik_link1(:,3),'b')
    
        ssp_ik_link2 = [ssp_ik_point1;ssp_ik_point2];
        plot3(ssp_ik_link2(:,1),ssp_ik_link2(:,2),ssp_ik_link2(:,3),'b')
    
        ssp_ik_link3 = [ssp_ik_point2;ssp_ik_point3];
        plot3(ssp_ik_link3(:,1),ssp_ik_link3(:,2),ssp_ik_link3(:,3),'b')
    
        ssp_ik_link4 = [ssp_ik_point3;ssp_ik_point4];
        plot3(ssp_ik_link4(:,1),ssp_ik_link4(:,2),ssp_ik_link4(:,3),'b')
    
        ssp_ik_link5 = [ssp_ik_point4;ssp_ik_point5];
        plot3(ssp_ik_link5(:,1),ssp_ik_link5(:,2),ssp_ik_link5(:,3),'b')
   
        %{
        plot3(link1(:,1),link1(:,2),link1(:,3),'b')
        plot3(link2(:,1),link2(:,2),link2(:,3),'b')
        plot3(link3(:,1),link3(:,2),link3(:,3),'b')
        plot3(link4(:,1),link4(:,2),link4(:,3),'b')
        plot3(link5(:,1),link5(:,2),link5(:,3),'b')
        %}
        
        pause(0.01)
    end
end

rectangle('Position',[-1 -1 2 2], 'FaceColor',[0 1 1])

axis([-1 1.5 -1 1 -0.1 0.6])

figure(2)
for i=1:j
    clf('reset')
    plot3(xcom(i),ycom(i),zcom(i),'.b')
    hold on
    
    rectangle('Position',[ssp_ik_x0(i)-(foot_length/2) ssp_ik_y0(i)-(foot_width/2) foot_length foot_width], 'FaceColor',[0 1 1])
    rectangle('Position',[ssp_ik_x5(i)-(foot_length/2) ssp_ik_y5(i)-(foot_width/2) foot_length foot_width], 'FaceColor',[0 1 1])
    
    axis([-0.1 0.4 -0.1 0.3 -0.1 0.5])
    pause(0.01)
end

%double stance case
theta = [pi/3; pi/2-0.0000001; -5*pi/6; -pi/6; pi/2-0.0000001; -2*pi/3]
alpha = [pi/2-0.0000001; -pi/2+0.0000001; -pi/2+0.0000001; pi/2-0.0000001]
a1 = 0.2 %step_length
a2 = 0.2 %distance between feet
xcom = xcom(1)
ycom = ycom(1)
zcom = zcom(1)

% we have 3 equations for alpha as functions of theta and one equation for
% alpha that we can choose ourselevs
% We have 4 equations for theta as functions of alpha and 2 equations as
% constraint equation
% reminder: due to convention issues, theta 5 and 6 are taken as negative
% of our convention

%equations for theta
C1 = l(2)*sin(alpha(1))
C2 = l(1)*sin(alpha(1))
C3 = l(1)*sin(alpha(4))
C4 = l(2)*sin(alpha(4))

D1 = m(2)*lc(2)*sin(alpha(1)) + m(1)*l(2)*sin(alpha(1)) + m(3)*l(2)*sin(alpha(1))
D2 = m(1)*lc(1)*sin(alpha(1)) + m(3)*l(1)*sin(alpha(1))
D3 = m(1)*lc(1)*sin(alpha(4))
D4 = m(2)*lc(2)*sin(alpha(4)) + m(1)*l(2)*sin(alpha(4))

xd = xcom*(total_mass) + a1*(m(1) + m(2)) - m(3)*bc*cos(theta(1)+theta(2)+theta(3)) %consider bc = 0 for easier calculations
zd = zcom*(total_mass) + m(3)*b*sin(alpha(1)+alpha(2))

P1 = (C3*xd - D3*a1)/(C2*D3 + C3*D2)
Q1 = (C1*D3 + C3*D1)/(C2*D3 + C3*D2)
R1 = (C4*D3 - C3*D4)/(C2*D3 + C3*D2)
S1 = (C3*zd + D3*b*sin(alpha(1)+alpha(2)))/(C2*D3 + C3*D2)

P2 = (-a1-C2*P1)/C3
Q2 = (C1 - C2*Q1)/C3
R2 = (C4 - C2*R1)/C3
S2 = (C2*S1 - b*sin(alpha(1)+alpha(2)))/(C3)

L1 = D1 - D2*Q1 + D3*Q2
M1 = D3*R2 - D2*R1 - D4

N1 = xd - D2*P1 + D3*P2
N2 = zd - D2*S1 - D3*S2

thetak = real(acos((N1^2 + N2^2 - L1^2 - M1^2)/(2*L1*M1)))

%theta(1) = atan2(((N2*L1 + N1*M1*sin(thetak) - N2*M1*cos(thetak))/(N1^2 + N2^2)), ((N1*L1 + N1*M1*cos(thetak) - N2*M1*sin(thetak))/(N1^2 + N2^2)))
theta(1) = real(acos(((N2*L1 + N1*M1*sin(thetak) - N2*M1*cos(thetak))/(N1^2 + N2^2))))

theta(6) = thetak - theta(1)

%theta(2) = atan2(S1 - Q1*sin(theta(1)) + R1*sin(theta(6)),P1 - Q1*cos(theta(1)) - R1*cos(theta(6)))
theta(2) = acos(S1 - Q1*sin(theta(1)) + R1*sin(theta(6)))

theta(3) = pi - theta(2) - theta(1)

%theta(5) = atan2(S2 + Q2*sin(theta(1)) - R2*sin(theta(6)), P2 - Q2*cos(theta(1)) - R2*cos(theta(6)))
theta(5) = acos(S2 + Q2*sin(theta(1)) - R2*sin(theta(6)))

theta(4) = pi - theta(5) - theta(6)

%equations for alpha

C1 = l(2)*sin(theta(1)) + l(1)*sin(theta(1)+theta(2))
C2 = l(2)*sin(theta(6)) + l(1)*sin(theta(5)+theta(6))

D1 = m(2)*lc(2)*sin(theta(1)) + m(1)*(l(2)*sin(theta(1)) + lc(1)*sin(theta(1)+theta(2))) + m(3)*l(2)*sin(theta(1))
D2 = m(2)*lc(2)*sin(theta(6)) + m(1)*(l(2)*sin(theta(6)) + lc(1)*sin(theta(5)+theta(6))) 

yd = ycom*total_mass - a2*(m(1)+m(2))

A1 = (m(3)*a2*bc - yd*b)/(m(3)*C1*bc - D1*b)
B1 = (m(3)*C2*bc + D2*b)/(m(3)*C1*bc - D1*b)

A2 = (a2 - C1*A1)/(b)
B2 = (C2 - C1*B1)/(b)

%equation for alpha4 needed here
alpha(4) = pi/2

alpha(1) = acos(A1 - B1*cos(alpha(4)))

alpha(2) = acos(A2 - B2*cos(alpha(4))) - alpha(1)

alpha(3) = -(alpha(1) + alpha(2) + alpha(4))



x0 = 0;
y0 = 0;
z0 = 0;
point0 = [x0,y0,z0];

x1 = l(2)*sin(alpha(1))*cos(theta(1));
y1 = l(2)*sin(theta(1))*cos(alpha(1));
z1 = l(2)*sin(theta(1))*sin(alpha(1));
point1 = [x1,y1,z1];

link1 = [point0;point1];

dx2 = l(1)*sin(alpha(1))*cos(theta(1)+theta(2));
dy2 = l(1)*sin(theta(1)+theta(2))*cos(alpha(1));
dz2 = l(1)*sin(theta(1)+theta(2))*sin(alpha(1));

x2 = x1 + dx2;
y2 = y1 + dy2;
z2 = z1 + dz2;
point2 = [x2,y2,z2];

link2 = [point1;point2];

dx3 = 0;
dy3 = b*cos(alpha(1)+alpha(2));
dz3 = b*sin(alpha(1)+alpha(2));

x3 = x2 + dx3;
y3 = y2 + dy3;
z3 = z2 + dz3;
point3 = [x3,y3,z3];

link3 = [point2;point3];

dx4 = l(1)*sin(alpha(4))*cos(theta(5)+theta(6));
dy4 = l(1)*sin(theta(5)+theta(6))*cos(alpha(4));90
dz4 = l(1)*sin(theta(5)+theta(6))*sin(alpha(4));

x4 = x3 + dx4;
y4 = y3 + dy4;
z4 = z3 + dz4;
point4 = [x4,y4,z4];

link4 = [point3;point4];

dx5 = l(2)*sin(alpha(4))*cos(theta(6));
dy5 = l(2)*sin(theta(6))*cos(alpha(4));
dz5 = l(2)*sin(theta(6))*sin(alpha(4));

x5 = x4 + dx5;
y5 = y4 + dy5;
z5 = z4 + dz5;
point5 = [x5,y5,z5];

link5 = [point4;point5];

figure(3)
hold on
plot3(link1(:,1),link1(:,2),link1(:,3),'b')
plot3(link2(:,1),link2(:,2),link2(:,3),'b')
plot3(link3(:,1),link3(:,2),link3(:,3),'b')
plot3(link4(:,1),link4(:,2),link4(:,3),'b')
plot3(link5(:,1),link5(:,2),link5(:,3),'b')
      
rectangle('Position',[-1 -1 2 2], 'FaceColor',[0 1 1])

axis([-1 1.5 -1 1 -0.1 0.6])
