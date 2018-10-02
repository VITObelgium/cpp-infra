%rio_poly2 specialized 2D polynomical trend function
%        
% y = rio_poly2( p, x )
% y = rio_poly2( p, x, x_lo, x_hi )
% 
%
% In case no additional arguments are given, the rio_poly2 function just
% returns the 2D polynomial from polyval, if given x_min and x_max,
% the 2D poly is kept constant outside these bounds, but never exceeding
% the parabola maximum. I.e. if a trend is presend which reaches the
% parabola macxium before the x_max variable, it will be kept constant
% from the maxium onwards...
%
% Author : Bino Maiheu, (c) VITO 2014
%
% See also rio_poly1

function y = rio_poly2( p, x, varargin )

if numel(p) ~= 3
  error( 'rio_poly2: number of parameters in p does not match' );
end

if nargin == 2
  y = polyval(p,x);
  return;
elseif nargin == 4
  x_lo = varargin{1};
  x_hi = varargin{2};
    
  if x_hi < x_lo
    error( 'rio_poly2: min/max indicators should be sorted' );
  end
else
  error( 'Wrong number of arguments' );
end
  
% This only get executed if the user provides additional arguments
% then just the polyfit parameters and x values
%-- Cast trend in parabolic form a.(x-b)^2+c
a =  p(1);
b = -p(2) ./ (2*a);
c =  p(3) - (p(2).^2)./(4*p(1));

y = a.*(x-b).^2+c;
      
val_lo = polyval( p, x_lo );
val_hi = polyval( p, x_hi );
        
% Depending on shape of parabola, make distinction
if a < 0
  % Concave parabola, a < 0
  % look on which edge we are, rising or falling with the st_indic
  if val_lo > val_hi
    % descending slope of concave parabola
    x_flat_lo = max(b, x_lo );
    x_flat_hi = x_hi;
  else
    % ascending slope of concave parabola
    x_flat_lo = x_lo;
    x_flat_hi = min(b,x_hi);
  end
elseif a >= 0
  % Convex parabola, a >= 0 (also for linezar case, but is just
  % exception
  if val_lo > val_hi
    % descending slope of convex parabola
    x_flat_lo = x_lo;
    x_flat_hi = min(b,x_hi);
  else
    % ascending slope of convex parabola
    x_flat_lo = max(b, x_lo );
    x_flat_hi = x_hi;
  end
end

% Keep the trend constand in the plateaus...
idx_flat_lo = find( x < x_flat_lo );
idx_flat_hi = find( x > x_flat_hi );
          
if any(idx_flat_lo),
  y( idx_flat_lo ) = a * (x_flat_lo - b)^2 + c;
end
        
if any(idx_flat_hi),
  y( idx_flat_hi ) = a * (x_flat_hi - b)^2 + c;
end
