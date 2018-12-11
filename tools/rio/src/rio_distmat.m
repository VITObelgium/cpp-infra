%RIO_DISTMAT
% Computes the distance matrix r_ij between the stations with coordinates
% x, where x [ x1 y1; x1 y2 ; ... ].  
%
% r_mat = rio_distmat( x )
% r_mat = rio_distmat( x, scale_to_km )
% 
% The optional scale argument scales the coordinates to a distance matrix
% in km, default is 0.001;
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function r_mat = rio_distmat( x, varargin )

n     = size( x, 1 );
scale = 0.001;
if nargin > 1
    scale = varargin{1};
end

% -- pre-allocate distance matrix
r_mat = zeros( n, n );

for i = 1:n
    for j = i:n
        r_mat(i,j) = scale .* sqrt( ( x(i,1) - x(j,1) ).^2 + (x(i,2) - x(j,2)).^2 );                
        r_mat(j,i) = r_mat(i,j);
    end
end