%RIO_COVMAT
% This one builds up the spatial correlation matrix for the interpolation
% of the current dataset, complemented with the extra row and column for the
% Kriging lagrange parameter. 
% 
% C_inv = rio_covmat( cnf, st_info )
%
% The st_info is an array with the station information, containing in the
% first column the station id and in columns 2 and 3 the x, y coordinates
% of the station, the number of stations is given by the number of rows in
% the st_info structure and this effectively also determines the size of
% the output inverse covariance matrix. 
%
% Note that the routine returns the inverse of this correlation matrix, not 
% the correlation matrix itself. 
%
% See also rio_init, rio_spatialcorr
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function C_inv = rio_covmat( cnf, st_info )

%-----------------------------------------------------------------
% calculate C_inv matrix from correlationmatrix between stations
%-----------------------------------------------------------------

%-- Convert coordinates from m to km
st_xy = st_info(:,[2,3]);
xi = st_xy(:,1)/1000.;
yi = st_xy(:,2)/1000.;

N = size(st_xy,1);
C = zeros( N );

%-- Calculate C-matrix for kriging, only compute half & copy... a bit
%-- faster & pre-allocate ;)
for i = 1:N
    for j = i:N
        % distance
        r_ij = sqrt((xi(i)-xi(j))^2 + (yi(i)-yi(j))^2);
        
        % matrix element
        C(i,j) = rio_spatialcorr( cnf, r_ij );        
        
        if ( i ~= j )
            C(j,i) = C(i,j);
        end
            
    end
end

%-- Add extra row and column for lagrange parameter 
C(:,N+1)=1;
C(N+1,:)=1;
C(N+1,N+1)=0;

%-- Inverse C-matrix
C_inv(:,:) = inv(C);

end
