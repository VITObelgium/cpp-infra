%RIO_SPATIALCORR
% This function returns the spatial correlation at inter-station distance
% d for the current RIO configuration, loaded in cnf (i.e. the current
% spatial correlation parameters set by rio_updatepars
%
% y = rio_spatialcorr( cnf, r )
%
% Notes:
%  - BM 2010-04-16 : kicked out the OrdKrig stuff, spatial correlation
%      function is of the same shape when using OrdKrig or RIO interpolation
%      the parameters are just based on the detrended data values. 
%      We can later on add more sophistication, however in no current
%      operational implementation we have used this, so keep things simple !
%
% See also rio_init, rio_covmat, rio_krige
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function y = rio_spatialcorr( cnf, r )

%-------------------------------------------------------
%-- Spatial correlation between 2 pollutant station as
%-- a function of the relative distance r [km] 
%-------------------------------------------------------     

%-- Cast trend in exponential form
A = exp(cnf.p_corr(2));
tau = -1. / cnf.p_corr(1);

%-- Cast trend in linear form
a = cnf.p_corr_short(1);
b = cnf.p_corr_short(2);

if length(r) > 1
    error('rio_spatialcorr:: error in spatial corr implementation!!' );
end

if r < 0.001
    y = 1.;
else
    y = max(max(A * exp(-r ./ tau), a * r + b), 0);
end

end