%rio_poly1 specialized 1D polynomical trend function
%        
% y = rio_poly1( p, x )
% y = rio_poly1( p, x, x_lo, x_hi )
% 
%
% In case no additional arguments are given, the rio_poly1 function just
% returns the 1D polynomial from polyval, if given x_lo and x_hi,
% the 1D poly is kept constant outside these bounds
%
% Author : Bino Maiheu, (c) VITO 2014
%
% See also rio_poly2

function fit = rio_poly1( p, x, varargin )

if numel(p) ~= 2
  error( 'rio_poly1: number of parameters in p does not match' );
end

fit = polyval(p,x);

if nargin == 2
  return;
elseif nargin == 4
  x_lo = varargin{1};
  x_hi = varargin{2};
  
  if x_hi < x_lo
    error( 'rio_poly1: min/max indicators should be sorted' );
  end
else
  error( 'Wrong number of arguments' );
end

val_lo = polyval( p, x_lo );
val_hi = polyval( p, x_hi );

fit( x < x_lo ) = val_lo;
fit( x > x_hi ) = val_hi;
