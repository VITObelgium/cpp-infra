%RIO_DISPLAY
% This routine makes a quick 'n dirty visualisation of a variable using the 
% current configuration which has the grid definitioni loaded in 
% cnf.grid_info
%
%  rio_display( cnf, value )
%  rio_display( cnf, value, missing )
%
% To build an easy configuration you can just to 
%  cnf = rio_griddef( X, Y, resol )
% 
% See also rio_init rio_griddef
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ scale ] = rio_display( cnf, value, varargin )

if nargin > 2
    missing = varargin{1};
else
    missing = -9999;
end

if length( value ) ~= cnf.grid_n
    error( 'Invalid grid length' );
end

% build raster
x      = min(cnf.grid_info(:,2)):cnf.grid_res:max(cnf.grid_info(:,2));
y      = max(cnf.grid_info(:,3)):-cnf.grid_res:min(cnf.grid_info(:,3));
[X,Y]  = meshgrid(x,y);
Z      = NaN( length(y), length(x) );
value( value == missing ) = NaN;

for i=1:cnf.grid_n      
    Z( ( X == cnf.grid_info(i,2) ) & ...
        ( Y == cnf.grid_info(i,3) ) ) = value(i);
end

pcolor( X, Y, Z );
shading flat;
