%RIO_EXCEEDPROBMC
% Calculates the exceedance probability for the given threshold for the 
% single pixel timeseries in val_ts and err_ts ( 1 sigma uncertainty )
% rho represents the correlation matrix between the time steps and should
% be of the same size as val_ts. 
% 
% [ P ] = rio_exceedprobmc( rho, val_ts, err_ts, limit, nthresh )
% [ P ] = rio_exceedprobmc( rho, val_ts, err_ts, limit, nthresh, n_ens )
%
% If the number of ensemble members is not given, a value of 500 is used... 
%
% WARNING: THIS ROUTINE IS HIGHLY EXPERIMENTAL !!
%
% See also rio_exceedances
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu


function P = rio_exceedprobmc( rho, val_ts, err_ts, limit, nthresh, varargin )

persistent C

if nargin > 5
    n_ens = varargin{1};
else
    n_ens = 500;
end

if ( size(rho,1) ~= size(rho,2) ) || ( size(rho,1) ~= length(val_ts) ) ...
        || ( size(rho,1) ~= length(err_ts) )
   error( 'rio_exceedprobmc:: invalid input array sizes, check code...\n' );
end

% Book C if necessary
if numel(C) ~= numel(rho)
    C = zeros( size(rho) );
end

% Rebuild covariance matrix but with correct errors, but keeping
% the correlation structure
for k=1:size(C,1)
    for l=1:size(C,2)
        C(k,l) = rho(k,l) .* sqrt( err_ts(k).^2 .* err_ts(l).^2 );
    end
end
        
% Compute covariance matrix eigenvalues
[D,lambda] = eig( C );

% means   = zeros( 1, n_ens );
excee   = zeros( 1, n_ens );
% ens_ts  = zeros( n_ens, length(val_ts) );
% ens_w   = zeros( n_ens, length(val_ts) );
        
for m=1:n_ens;
    r = randn(length(val_ts), 1 );
    v = sqrt(lambda) * r;
    w = D*v;  % 0 mean correlated white noise with required covariance
    
    % ens_w(m,:) = w;
    
    % Add the value again...
    w = w + val_ts';
    
    % means(m) = mean( w );
    excee(m) = length( find( w > limit ) );
    
    % Store the ensemble for validation...
    % ens_ts(m,:) = w;
    
end

% Calculate exceedance probability of ndays norm for number of exceedances
x = length( find( excee > nthresh ) );
P = x ./ n_ens;


end