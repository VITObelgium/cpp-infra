%RIO_CALCFCOV
% Calculates covariance correction factor from the gridded dataset, also
% returns the correlation matrix...
%
%  [ fcov ] = rio_calcfcov( grid_all )
%
% See also rio_average
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ fcov ] = rio_calcfcov( grid_all )

N = size( grid_all, 2 );
C = cov( grid_all ); 
V = var( grid_all );

% Calculate some intermediate terms
covSum = sum( reshape(C,[1 numel(C)]) ) - trace(C); % off-diagonal sum
varSum = sum(V);

% Correction factor for autocorrelation
fcov = 1./N * ( 1. + covSum / varSum );
end