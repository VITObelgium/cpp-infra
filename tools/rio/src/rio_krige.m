%RIO_KRIGE
% Krige to a single grid cell. This function calculates the vector with
% spatial correlations between the stations and the x,y grid location and
% computes the kriging weights from the inverted covariance matrix. 
% The interpolated value and the kriging error variance are returned. 
%  
%  [ val, err ] = rio_krige( cnf, C_inv, st_info, xx_data, x, y )
%
% See also rio_init, rio_spatialcorr, rio_covmat, rio_dblookup, rio_krigegrid
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ krig_val, krig_err ] = rio_krige( cnf, C_inv, st_info, xx_data, x, y )

%-- We take the variance of the detrended values to be the
%   sill of the variogram in the error calculation even though
%   the spatial correlations are established over long terms
%   See: RIO 2009 report, is calculated in the krig_ipol routine...


%--------------------
% Perform Kriging
%--------------------

st_x = st_info(:,2)/1000.;
st_y = st_info(:,3)/1000.;

st_n = size(st_info,1);

%-- Calculate D array for kriging:
%-- Covariance between station and location 
D = zeros( 1, st_n );
for i = 1:st_n
    r_ij = sqrt((st_x(i)-x)^2 + (st_y(i)-y)^2);
    D(i) = rio_spatialcorr( cnf, r_ij );
end
D(st_n + 1) = 1.;

%-- Calculate weights array
w = C_inv * D';

%-----------------------------
%-- Calculate Kriging Value...
%-----------------------------
%-- take the variance of the input data as the variance of the random
%   function model, in the RIO interpolation case, this is the variance
%   of the detrended data, in the OrdKrig case, this is just the variance
%   of the data...
xx_var =  var( xx_data(:, 2) );


%-- If we don't use the logtrans option, keep this in for backward
%   compatibility... always have found this somewhat bizarre...
if ~cnf.Option.logtrans
  %-- Log transformation (NOT for O3!!)...
  if strcmp( cnf.pol_xx, 'no2') || strcmp( cnf.pol_xx, 'pm10') ...
      || strcmp( cnf.pol_xx, 'so2' ) || strcmp( cnf.pol_xx, 'pm25' )
    xx_data(:,2) = log(1 + xx_data(:,2));
    %-- recalculate the variance for the log transformed data...
    xx_var = var( xx_data(:,2) );
  end
end

%-- Calculate output from lin combination
krig_val = xx_data(:,2)' * w(1:st_n);
krig_var = xx_var * ( 1 - w' * D' );

%-- If we don't use the logtrans option, keep this in for backward
%   compatibility... always have found this somewhat bizarre...
if ~cnf.Option.logtrans
  %-- Retransform log distribution...
  if strcmp(cnf.pol_xx, 'no2') || strcmp(cnf.pol_xx, 'pm10') ...
      || strcmp(cnf.pol_xx, 'so2') || strcmp( cnf.pol_xx, 'pm25' )
    krig_var = exp(2*krig_val) .* krig_var;
    krig_val = exp(krig_val) - 1;
  end
end

krig_err = sqrt( krig_var );
end

        

