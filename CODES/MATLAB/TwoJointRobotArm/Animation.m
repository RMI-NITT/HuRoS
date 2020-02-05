[size_rowSS size_colSS] = size(simth1);

%initial positions
th10 = -pi/2;
th20 = 0;
th30 = (th10+th20);

l = 0.4;

x0 = 0;
y0 = 0;

x2 = l*cos(th10);
y2 = l*sin(th10);

x4 = x2 + l*cos(th30);
y4 = y2 + l*sin(th30);

figure;

axesHandle = gca;

xlim(axesHandle, [(-1) (1)]);
ylim(axesHandle, [(-1) (1)]);

rectHandle0 = rectangle('Position',[x0 y0 .05 .05],  'Curvature',[1,1],'FaceColor','b');%link segment ends
rectHandle1 = rectangle('Position',[x2 y2 .05 .05],  'Curvature',[1,1],'FaceColor','g');%link segment ends

hold on

lineHandle1 = line([x2 x0],[y2 y0]); %link segments
lineHandle2 = line([x4 x2],[y4 y2]); %link segments

set(lineHandle1,'Color','b');   %line 1 color
set(lineHandle2,'Color','g');   %line 2 color

hold off

for j=1:size_rowSS
   
    drawnow; %Forces MATLAB to render the snake
    
    %%recalculate positions - 
    
    x0 = 0;
    y0 = 0;

    x2 = (l)*cos(simth1(j,1));
    y2 = (l)*sin(simth1(j,1));

    x4 = x2 + (l)*cos((simth1(j,1))+(simth2(j,1)));
    y4 = y2 + (l)*sin((simth1(j,1))+(simth2(j,1)));
    
    %redraw links
    set(rectHandle1,'Position',[x2 y2 .05 .05]);
    
    %redraw snake
    set(lineHandle1,'XData',[x2 x0],'YData',[y2 y0]);
    set(lineHandle2,'XData',[x4 x2],'YData',[y4 y2]);
    
    pause(0.005)
    
end