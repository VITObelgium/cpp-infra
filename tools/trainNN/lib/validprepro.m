%VALIDPREPRO Validation preprocessor routine
%
%
% [ have_index, o, m ]        = validprepro( mod, obs, varargin )
% [ have_index, o, m, oo, mm] = validprepro( mod, obs, varargin )
%
% mod/obs can either be 1D arrays or cell arrays containing sets of
% matching 1D arrays. in case we have 2D arrays, we assume the first
% column (or row) is an index : i.e. either a timestamp, or some other
% monotonically increasing value. (e.g. station number or so)
%
% Parameters
%
%  'missingValue', <val> ...... : use this as mising value instread of NaN
%  'interpolTime', <t/f> ...... : interpolate the model values to the 
%                                 observation, we assume an index array
%                                 is present
%
% See also validstats, targplot
%
% Bino Maiheu (bino.maiheu@vito.be)
% Copyright 2012-2014, Flemish Instititute for Technological Research

function [ have_index, o, m, varargout ] = validprepro( mod, obs, varargin )

p = inputParser;
p.CaseSensitive = true;
p.addParamValue( 'missingValue', NaN, @isnumeric );
p.addParamValue( 'interpolTime', false, @islogical );
p.parse( varargin{:} );
par = p.Results;

if xor( iscell(mod),  iscell(obs) )
  error('MATLAB:validprepro', 'Either both cell arrays or not' );
end
if iscell(mod)
  if any( size(mod)-size(obs) )
    error( 'MATLAB:validprepro', 'Cell sizes differ...' );
  end
  
  % advance booking
  have_index = cell(size(mod));
  o          = cell(size(mod));
  m          = cell(size(mod));
  oo         = cell(size(mod));
  mm         = cell(size(mod));
  
  for k=1:numel(mod)
    [ have_index{k}, o{k}, m{k}, oo{k}, mm{k} ] = prepro( mod{k}, obs{k}, par );    
  end
  
else
  [have_index, o, m, oo, mm ] = prepro( mod, obs, par );
end

if nargout == 5
    varargout{1} = oo;
    varargout{2} = mm;
end

    
  
function [ have_index, o, m, varargout ] = prepro( mod, obs, p )

% do we have a time/index array ?
have_index = false;

% treat the mod/obs arrays, first make sure the lowest dimension of the
% datapoint array is 3, so if a dimension is 2, it is becaue of hte time
% array
if max(size(obs)) < 3 || max(size(mod)) < 3
  error( 'MATLAB:validprepro', 'Need at least 3 model values/observations for validation' );
end
if min(size(obs)) == 1 && min(size(obs)) == 1
  if ( length(obs) ~= length(mod) )
    error( 'MATLAB:validprepro', 'Equal length required for 1D validation arrays' );
  end
  % we have both Nx1D arrays
  o = obs(:);
  m = mod(:);

elseif min(size(obs)) == 2 && min(size(obs)) == 2
  % we have both Nx2D arrays, transpose when needed
  if size(obs,1) == 2, obs = obs'; end;
  if size(mod,1) == 2, mod = mod'; end;
  
  % check if the "time" or station array is complete
  if any( isnan( obs(:,1) ) ) || any( obs(:,1) == p.missingValue ) 
    error( 'MATLAB:validprepro', 'Observation time/idx array contains missing values' )
  end
  if any( isnan( mod(:,1) ) ) || any( mod(:,1) == p.missingValue ) 
    error( 'MATLAB:validprepro', 'Model time/idx array contains missing values' )
  end
  
  % check if time array is unique
  if length(unique(obs(:,1))) ~= length(obs(:,1))
    error( 'MATLAB:validprepro', 'Observation array time/idx array contains duplicates' );
  end
  if length(unique(mod(:,1))) ~= length(mod(:,1))
    error( 'MATLAB:validprepro', 'Model array time/idx array contains duplicates' );
  end
  
  % intersect or interpolate
  if p.interpolTime
    o = obs(:,2);
    m = interp1( mod(:,1), mod(:,2), obs(:,1), 'linear' );
  else
    [ tmp, io, im ] = intersect( obs(:,1), mod(:,1) );
    o = obs(io,2);
    m = mod(im,2);
  end

  have_index = true;
else
  error( 'MATLAB:validprepro', 'Observation/model array dimensions are not consistent' );
end
  
% oo and mm are unmatched --> contain the full dataset, without
% the mising values for separate statistics
oo = o; oo( oo == p.missingValue | isnan(oo) ) = [];
mm = m; mm( mm == p.missingValue | isnan(mm) ) = [];

% Now we should have 2 matched pairwise 1D arrays. First strip of the
% missing values in both before continuing...
i_missing = find( o == p.missingValue | ...
  m == p.missingValue | isnan(o) | isnan(m) );

% o and m are matched and of equal size --> pairwise validation
o( i_missing ) = [];
m( i_missing ) = [];

% output the 
if nargout == 5
  varargout{1} = oo;
  varargout{2} = mm;
end
