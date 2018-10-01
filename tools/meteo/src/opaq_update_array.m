% opaq_update_array Updates a time array with new values, if not missing that is...
% 
%  [ xx_date, xx_value ] = opaq_update_array( old_date, old_value, new_date, new_value, varargin )
%
%  Optional parameters
%  
%  'missing', <value>   : use this as missing value, default is NaN
%
% Author: Bino Maiheu, (c) 2013-2014 VITO

function [ xx_date, xx_value ] = opaq_update_array( old_date, old_value, new_date, new_value, varargin )

% -- some input parameter parsing...
ip = inputParser();
addRequired(ip, 'old_date', @isvector );
addRequired(ip, 'old_value', @isvector );
addRequired(ip, 'new_date', @isvector );
addRequired(ip, 'new_value', @isvector );
addParamValue( ip, 'missing', NaN );

parse(ip,old_date, old_value, new_date, new_value,varargin{:});
opts = ip.Results;

% -- Convert to colum vector...
old_date  = old_date(:);
old_value = old_value(:);
new_date  = new_date(:);
new_value = new_value(:);

%-- Some checks on the date arrays for contiguousness and timestep
do = diff( old_date );
dn = diff( new_date );
if ~all(do==do(1)), error( 'opaq_update_array: old array is not contiguous...' ); end;
if ~all(dn==dn(1)), error( 'opaq_update_array: new array is not contiguous...' ); end;
if do(1) ~= dn(1),  error( 'opaq_update_array: different timestep in old/new array...' ); end;

%-- Check is the datesequences are similarily timed, just construct the
%   series anew
all_dates = [ old_date; new_date ];
xx_date  = min( all_dates ):do(1):max(all_dates); xx_date = xx_date';
xx_value = opts.missing * ones( size(xx_date) );

%-- Construct an expanded array of dates
[ ~, ia, ib ] = intersect( xx_date, old_date );
xx_value(ia) = old_value(ib); % set the old values

%-- now put the new values if they are not missing
for k=1:length( new_date )
  i = find( abs( xx_date - new_date(k) ) < 1e-6 );
  if isempty( i ) % should not happen since we created a union
    error( 'opaq_update_array: Fatal error in date array union...' );
  end
  
  if ~( isnan( new_value(k) ) || new_value(k) == opts.missing )
    xx_value(i) = new_value(k);
  end
end