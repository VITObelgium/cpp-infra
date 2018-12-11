%RIO_SHOWGRID
% This routine makes a quick 'n dirty visualisation of the grid using the 
% current configuration which has the grid definitioni loaded in 
% cnf.grid_info
%
%  rio_showgrid( cnf, grid, scale )
%
%  The routine uses a colormap defined by 10 colors and limits set by the 
%  scale array with 10 elements defining 10 bins ( the last value is
%  considered to be the lower limit of a bit with no maximum value : 
%  cfr. pm10 > 50 µg/m3 ). 
%
% See also rio_init
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ scale ] = rio_showgrid( cnf, grid, varargin )

if nargin > 2 
   scale = varargin{1};
else
   % create the scale ourselves based upon the hist function...
   % We use the matlab hist function to get some sort of natural breaks...
   [ n, breaks ] = hist( grid(:,2), 11 );
   breaks = breaks - .5*(breaks(2)-breaks(1)); % Need the left edges, not centers
   scale  = breaks(1:10);
   scale(1) = 0; % and reset the first to start from 0.
end   

x      = min(cnf.grid_info(:,2)):cnf.grid_res:max(cnf.grid_info(:,2));
y      = max(cnf.grid_info(:,3)):-cnf.grid_res:min(cnf.grid_info(:,3));
[X,Y]  = meshgrid(x,y);
Z      = -9998*ones( length(y), length(x) );
colZ   = zeros(size(Z));

for i=1:cnf.grid_n      
    Z( ( X == cnf.grid_info(i,2) ) & ...
        ( Y == cnf.grid_info(i,3) ) ) = grid(i,2);
end

% define colormap for plot, thanks jef
map(1,1:3)  = [0 0 255]./255;
map(2,1:3)  = [0 126 253]./255;
map(3,1:3)  = [0 192 0]./255;
map(4,1:3)  = [0 255 0]./255;
map(5,1:3)  = [204 255 51]./255;
map(6,1:3)  = [248 231 72]./255;
map(7,1:3)  = [255 128 0]./255;
map(8,1:3)  = [255 0 0]./255;
map(9,1:3)  = [192 0 0]./255;
map(10,1:3) = [128 0 0]./255;

% points outside the interpolation grid
map(11,1:3)=[0.6 0.6 0.6];
% error code -9999
map(12,1:3)=[1 1 1];

for i=1:numel(Z), 
    colZ(i) = map_code( scale, Z(i) );
end;

imagesc(colZ, [1 12]); 
colormap(map);


function y = map_code( scale, x )

n = length(scale);
for i=1:n-1
   if ( ( x >= scale(i) ) && ( x < scale(i+1) ) ) 
       y = i; 
       return;
   end
end
if (x >= scale(n) ), y = n; return; end;
  
% points outside the grid
if ( x == -9998 ),    y=n+1; return; end;
% points with error code
if ( x == -9999 ),    y=n+2; return; end;


