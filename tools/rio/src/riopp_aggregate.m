% riopp_aggregate
%
% Aggregates the data grid in which the first dimension is the 
% grid index and the second dimension is the time index by the 
% statistic given. 
%
% [ v_aggr ] = riopp_aggregate( v, stat )
% [ v_aggr ] = riopp_aggregate( v, stat, period )
%
% The default period is 24, meaning that we calculate the statistic
% for every 24 values (time index). Note that the number of timeindices
% must be dividably by the period !
%
% Available aggregations are : 
%  - mean    : average
%  - std0    : standard deviation normalized by N-1(unbiased estimator for sample variance)
%  - std1    : standard deviations normalized by N 
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu
function [ v_aggr ] = riopp_aggregate( stat, v, varargin )

P = 24;  % aggregeation period
if nargin > 2
    P = varargin{1};
end

% check dimension
if mod(size(v,2),P) ~= 0
    error( 'riopp_aggregate: time dimension not multiple of the period' );
end

switch ( stat )
    case 'mean'        
        v_aggr = squeeze( mean( reshape( v, size(v,1), 24, size(v,2)/24 ), 2 ) );
    case 'nanmean'        
        v_aggr = squeeze( nanmean( reshape( v, size(v,1), 24, size(v,2)/24 ), 2 ) );
    case 'std0'
        v_aggr = squeeze( std( reshape( v, size(v,1), 24, size(v,2)/24 ), 0, 2 ) );
    case 'std1'
        v_aggr = squeeze( std( reshape( v, size(v,1), 24, size(v,2)/24 ), 1, 2 ) );
    otherwise
        error( 'unknown statistic : %s', statistic );
end

function y = nanmean(x,dim)
% FORMAT: Y = NANMEAN(X,DIM)
% 
%    Average or mean value ignoring NaNs
%
%    This function enhances the functionality of NANMEAN as distributed in
%    the MATLAB Statistics Toolbox and is meant as a replacement (hence the
%    identical name).  
%
%    NANMEAN(X,DIM) calculates the mean along any dimension of the N-D
%    array X ignoring NaNs.  If DIM is omitted NANMEAN averages along the
%    first non-singleton dimension of X.
%
%    Similar replacements exist for NANSTD, NANMEDIAN, NANMIN, NANMAX, and
%    NANSUM which are all part of the NaN-suite.
%
%    See also MEAN

% -------------------------------------------------------------------------
%    author:      Jan Gläscher
%    affiliation: Neuroimage Nord, University of Hamburg, Germany
%    email:       glaescher@uke.uni-hamburg.de
%    
%    $Revision: 1.1 $ $Date: 2004/07/15 22:42:13 $

if isempty(x)
	y = NaN;
	return
end

if nargin < 2
	dim = min(find(size(x)~=1));
	if isempty(dim)
		dim = 1;
	end
end

% Replace NaNs with zeros.
nans = isnan(x);
x(isnan(x)) = 0; 

% denominator
count = size(x,dim) - sum(nans,dim);

% Protect against a  all NaNs in one dimension
i = find(count==0);
count(i) = ones(size(i));

y = sum(x,dim)./count;
y(i) = i + NaN;