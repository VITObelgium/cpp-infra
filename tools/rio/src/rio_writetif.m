%RIO_WRITETIF 
% Easy RIO geotiff writer
%
% rio_writetif( fname, X )
% rio_writetif( fname, X, nodata )
% rio_writetif( fname, X, nodata, griddef )
%
% Currently, it automatically selects the 4x4 grid and only supports
% this...
% 
% need addpath D:\Matlab\MatlabToolkit\IO
%
% See also rio_init rio_griddef
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_writetif( fname, value, varargin )

grd = rio_griddef;

nodata = -9999;
if nargin > 2
    nodata = varargin{1};
    if nargin > 3
        grd = varargin{2};
    end
end

xx  = grd.grid_info(:,2);
yy  = grd.grid_info(:,3);
res = grd.grid_res;

% build raster
edge   = 4*res;
x      = min(xx)-edge:res:max(xx)+edge;
y      = max(yy)+edge:-res:min(yy)-edge;
[X,Y]  = meshgrid(x,y);
Z      = NaN( length(y), length(x) );

for i=1:size(xx,1)      
    Z( ( X == xx(i) ) & ( Y == yy(i) ) ) = value(i);
end

geo = struct( ...
    'GTModelTypeGeoKey', 1, ...
    'ModelPixelScaleTag',  [ res; res; 0 ], ...
    'ModelTiepointTag', [ 0.5 0.5 0 x(1) y(1) 0], ...
    'ProjectedCSTypeGeoKey', 31370, ... 
    'PCSCitationGeoKey', 'Belge 1972 / Belgian Lambert 72', ...
    'NaN', nodata );


% export geotiff
geotiffwrite(  fname, [], Z, 32, geo );
