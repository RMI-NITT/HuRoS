%parameters
l = [ 1; 1]

m = [1; 1; 0.3]

theta = [ pi/3; pi/2; pi/2; 0; 0; -pi*(5/6)]

alpha = [ pi/2; -pi/2; -pi/2; pi/2]

b = 0.2

x1 = l(2)*sin(alpha(1))*cos(theta(1))
y1 = l(2)*sin(theta(1))*cos(alpha(1))
z1 = l(2)*sin(theta(1))*sin(alpha(1))

link1 = line([0;l(2)*sin(alpha(1))*cos(theta(1))],[0;l(2)*sin(theta(1))*cos(alpha(1))],[0; l(2)*sin(theta(1))*sin(alpha(1))])

dx2 = l(1)*sin(alpha(1))*cos(theta(1)+theta(2))
dy2 = l(2)*sin(theta(1)+theta(2))*cos(alpha(1))
dz2 = l(2)*sin(theta(1)+theta(2))*sin(alpha(1))

x2 = x1 + dx2
y2 = y1 + dy2
z2 = z1 + dz2

link2 = line([x1;x2],[y1;y2],[z1;z2])

dx3 = 0
dy3 = b*cos(alpha(1)+alpha(2))
dz3 = b*sin(alpha(1)+alpha(2))

x3 = x2 + dx3
y3 = y2 + dy3
z3 = z2 + dz3

link3 = line([x2;x3],[y2;y3],[z2;z3])

dx4 = l(1)*sin(alpha(4))*cos(theta(5)+theta(6))
dy4 = l(1)*sin(theta(5)+theta(6))*cos(alpha(4))
dz4 = l(1)*sin(theta(5)+theta(6))*sin(alpha(4))

x4 = x3 + dx4
y4 = y3 + dy4
z4 = z3 + dz4

link4 = line([x3;x4],[y3;y4],[z3;z4])

dx5 = l(2)*sin(alpha(4))*cos(theta(6))
dy5 = l(2)*sin(theta(6))*cos(alpha(4))
dz5 = l(2)*sin(theta(6))*sin(alpha(4))

x5 = x4 + dx5
y5 = y4 + dy5
z5 = z4 + dz5

link4 = line([x4;x5],[y4;y5],[z4;z5])
view(3)
