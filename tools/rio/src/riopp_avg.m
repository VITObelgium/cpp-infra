%riopp_average Computes concentration averages
%
%
% [avg_grid, avg_st ] = riopp_avg( C, U, obs, norm )
% [avg_grid, avg_st ] = riopp_avg( C, U, obs, norm, fcov )
%
%
% Output matrices contain colmnvectors : 
%
% avg_grid : <cnt> <avg> <avg_err> <stddev> <excee prob>
% avg_st   : <cnt> <avg> <stddev>
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ avg_grid, avg_st ] = riopp_avg( C, U, obs, eu_norm, varargin )

if nargin > 4
    fcov = varargin{1};
else
    fcov = 1;
end

avg_grid      = zeros( size(C,1), 6 );
avg_st        = zeros( size(obs,1), 3 );

% Average the grid
avg_grid(:,1) = nancount( C, 2 );
avg_grid(:,2) = nanmean( C, 2 );
avg_grid(:,3) = sqrt( fcov * nansum(U.^2,2)./avg_grid(:,1) );
avg_grid(:,4) = nanstd( C, 2, 1 ); % normalise by N-1 (last flag=0)

%-- For the stations... (division by 0 gives NaN)
avg_st(:,1) = nancount( obs, 2 );
avg_st(:,2) = nanmean( obs, 2 );
avg_st(:,3) = nanstd( obs, 2, 0 );

if eu_norm > 0.   
   avg_grid(:,5) = 100. * ( 1. - rio_normcdf( eu_norm, avg_grid(:,3), avg_grid(:,4) ) );         
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

function y = nanstd(x,dim,flag)
% FORMAT: Y = NANSTD(X,DIM,FLAG)
% 
%    Standard deviation ignoring NaNs
%
%    This function enhances the functionality of NANSTD as distributed in
%    the MATLAB Statistics Toolbox and is meant as a replacement (hence the
%    identical name).  
%
%    NANSTD(X,DIM) calculates the standard deviation along any dimension of
%    the N-D array X ignoring NaNs.  
%
%    NANSTD(X,DIM,0) normalizes by (N-1) where N is SIZE(X,DIM).  This make
%    NANSTD(X,DIM).^2 the best unbiased estimate of the variance if X is
%    a sample of a normal distribution. If omitted FLAG is set to zero.
%    
%    NANSTD(X,DIM,1) normalizes by N and produces the square root of the
%    second moment of the sample about the mean.
%
%    If DIM is omitted NANSTD calculates the standard deviation along first
%    non-singleton dimension of X.
%
%    Similar replacements exist for NANMEAN, NANMEDIAN, NANMIN, NANMAX, and
%    NANSUM which are all part of the NaN-suite.
%
%    See also STD

% -------------------------------------------------------------------------
%    author:      Jan Gläscher
%    affiliation: Neuroimage Nord, University of Hamburg, Germany
%    email:       glaescher@uke.uni-hamburg.de
%    
%    $Revision: 1.1 $ $Date: 2004/07/15 22:42:15 $

if isempty(x)
	y = NaN;
	return
end

if nargin < 3
	flag = 0;
end

if nargin < 2
	dim = min(find(size(x)~=1));
	if isempty(dim)
		dim = 1; 
	end	  
end


% Find NaNs in x and nanmean(x)
nans = isnan(x);
avg = nanmean(x,dim);

% create array indicating number of element 
% of x in dimension DIM (needed for subtraction of mean)
tile = ones(1,max(ndims(x),dim));
tile(dim) = size(x,dim);

% remove mean
x = x - repmat(avg,tile);

count = size(x,dim) - sum(nans,dim);

% Replace NaNs with zeros.
x(isnan(x)) = 0; 


% Protect against a  all NaNs in one dimension
i = find(count==0);

if flag == 0
	y = sqrt(sum(x.*x,dim)./max(count-1,1));
else
	y = sqrt(sum(x.*x,dim)./max(count,1));
end
y(i) = i + NaN;

function y = nancount(x,dim)
% FORMAT: Y = NANCOUNT(X,DIM)
% 
%    Counts the number of non-NaN values 
%

if isempty(x)
	y = 0;
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
y = size(x,dim) - sum(nans,dim);

function y = nansum(x,dim)
% FORMAT: Y = NANSUM(X,DIM)
% 
%    Sum of values ignoring NaNs
%
%    This function enhances the functionality of NANSUM as distributed in
%    the MATLAB Statistics Toolbox and is meant as a replacement (hence the
%    identical name).  
%
%    NANSUM(X,DIM) calculates the mean along any dimension of the N-D array
%    X ignoring NaNs.  If DIM is omitted NANSUM averages along the first
%    non-singleton dimension of X.
%
%    Similar replacements exist for NANMEAN, NANSTD, NANMEDIAN, NANMIN, and
%    NANMAX which are all part of the NaN-suite.
%
%    See also SUM

% -------------------------------------------------------------------------
%    author:      Jan Gläscher
%    affiliation: Neuroimage Nord, University of Hamburg, Germany
%    email:       glaescher@uke.uni-hamburg.de
%    
%    $Revision: 1.2 $ $Date: 2005/06/13 12:14:38 $

if isempty(x)
	y = [];
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

% Protect against all NaNs in one dimension
count = size(x,dim) - sum(nans,dim);
i = find(count==0);

y = sum(x,dim);
y(i) = NaN;