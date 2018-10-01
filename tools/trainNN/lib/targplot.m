%TARGPLOT produce a targetplot
%
% targplot(x,y)
% targplot(mod, obs, 'inputMode', 'raw')
%
%
% Produces a target plot, this can be constructed in diferent ways :
%
% 1. If one already has the coorindates for the target plot values and just
%    wants to produce a plot, then you can all the function just
%    
%      targplot(x,y);
%  
%    where x and y are equally sizes arrays of target coordinates, which
%    will be put on the plot. the targplot routine assumes the coordinates
%    are in column vectors, so everey column will correspond to a category,
%    for which a different symbol is used. you can specify this with the 
%    optional 'catSymbol' parameter. 
% 
%    if in each category, a different number of points are contained, than
%    you can also specify the x y coordinates by means of a cell array of
%    equal size, where in each cell element an array is contained with the
%    x or y coordinates for that category, e.g.
%
%      targplot( { [ .25 .3 -1.2 ], [ .8 0.7 ], [0.3 ] }, ...
%                { [ .71 .4 0.2],   [ .2 0.5 ], [1.5 ] }, ...
%                'catSymbol', { 'go', 'k.', 'sb' } );
%
% 2. On the other hand, the routine also supports the calculation of the
%    target plot values themselves when provided with model
%    observation pair arrays. In this case we have to specify the optional
%    parameter inputMode and set it to 'raw'
%
%      targplot( mod, obs, 'inputMode', 'raw' );
%
%    when the mod/obs arrays are numerical arrays, then the routine
%    assumes the model and observation infor are in column vectors, the
%    differnet columns are then different mod/obs pairs timeseries, and
%    will end up as different points of the same category (only 1 category
%    posisbly then) on the same plot. 
%  
%    However, if you specify a 1D mod/obs cell arrays. In order
%    to specify a category to which they belong, one has to specify an 
%    array in the 'catIndex' parameter of the same number of elements
%    as the mod,obs cell arrays, in these index arrays the indix for the 
%    category symbol and labels are contained.Note that the mod/obs
%    arguments can also be numerical arrays
%
%
% Parameters
% 'inputMode', 'name' ....... : can be 'raw' or 'coords', the latter case
%                                 we assume the input array contains
%                                 already the coordinates to plot. coords
%                                 is the default
% 'missingValue', <val> ..... : alternative missing value other than NaN.
%
% 'interpolTime', <t/f> ..... : instead of intersecting the time arrays in 
%                               obs/mod matrices, we interpolate linearly 
%                               the model times to the observations to
%                               match.
%
% 'targetMax', <val> ........ : maximum for target plot, def. 2.5
%
% 'catSymbol', {...} ........ : cell array of custom plotting symbols for 
%                               each category
% 'catLabel', {...} ......... : cell array of category labels
% 'catIndex', [ ... ] ....... : 1d array with category indices for the list
%                               of mod/obs cells (only necessary with 
%                               inputMode raw, if not we assume all mod/obs
%                               cell pairs are of the same category). Note,
%                               when given should be of the same number of
%                               elements as mod/obs.
% 'Color', [ r g b ] ........ : specify the color for the shaded area
% 
%
% Bino Maiheu (bino.maiheu@vito.be)
% Copyright 2012-2014, Flemish Instititute for Technological Research

function targplot(x,y,varargin)

%-- parsing arguments
p = inputParser;
expectedInputModes = {'coords','raw'};

p.addParamValue( 'inputMode', 'coords',  @(x) any(validatestring(x,expectedInputModes)) );
p.addParamValue( 'missingValue', NaN, @isnumeric );
p.addParamValue( 'interpolTime', false, @islogical );
p.addParamValue( 'targetMax', 2.5, @isnumeric );
p.addParamValue( 'catSymbol', { 'k.', 'b.', 'r.', 'g.', 'c.' }, @iscell );
p.addParamValue( 'catLabel',  {}, @iscell );
p.addParamValue( 'catIndex', [], @isnumeric );
p.addParamValue( 'Color', [153 255 51]./255, @isnumeric );
p.parse( varargin{:});

% check if we have enough plotting symbols for each category
if any( size(x) - size(y) )
  error( 'MATLAB:targplot', 'Input sizes for x,y coordinates not equal...' );
end

%-- build the targ_x, targ_y arrays for plotting
if strcmp( p.Results.inputMode, 'raw' )
  
  if isnumeric(x) && isnumeric(y)
    
    tx = nan(size(x,2),1);
    ty = nan(size(x,2),1);
    
    for k=1:size(x,2)
      [ have_index, o, m, oo, mm ] = validprepro( x(:,k), y(:,k), ...
        'missingValue', p.Results.missingValue, 'interpolTime', p.Results.interpolTime );
      
      [tx(k), ty(k)] = target( o, m, oo, mm );
  
    end
    % assign all the columns to the same category
    targ_x{1} = tx;
    targ_y{1} = ty;
    
  elseif iscell(x) && iscell(y)
    
    % we have a cell arrays, now lookup the categories and put them into the    
    % targ_x and targ_y cell arrays for using the same plotting routine..
    if isempty( p.Results.catIndex )
      tx = nan(numel(x),1);
      ty = nan(numel(x),1);
      % all in same category
      for k=1:numel(x)
        [ have_index, o, m, oo, mm ] = validprepro( x{k}, y{k}, ...
          'missingValue', p.Results.missingValue, 'interpolTime', p.Results.interpolTime );
      
        [tx(k), ty(k)] = target( o, m, oo, mm );
      end
      targ_x{1} = tx;
      targ_y{1} = ty;
    else
      if any( size(p.Results.catIndex) - size(x) ) || ...
          any( size(p.Results.catIndex) - size(x) ) 
        error( 'MATLAB:targplot', 'Expect equal size of catIndex and input mod/obs...' );
      end
            
      % We assume that the catIndex contains the indices for the targets,
      % so we can assume the maximum index number is the required size of 
      % the array
      % 
      % mod = { [....], [ .... ], [ ....] , [.....] }
      % obs = { [....], [ .... ], [ ....] , [.....] }
      % catIdexn = [ 1 1 2 2 ];
      targ_x = cell(1,max(p.Results.catIndex));
      targ_y = cell(1,max(p.Results.catIndex));
            
      for k=1:numel(x)
        
        [ have_index, o, m, oo, mm ] = validprepro( x{k}, y{k}, ...
          'missingValue', p.Results.missingValue, 'interpolTime', p.Results.interpolTime );
        
        [tx, ty ]  = target( o, m, oo, mm );
      
        targ_x{p.Results.catIndex(k)}(end+1) = tx;
        targ_y{p.Results.catIndex(k)}(end+1) = ty;
      end
    end  
  else
    error( 'MATLAB:targplot', 'mod/obs should either be both cell or numeric' );
  end
  
else % input are already coordinate lists
    
  if size(x,2) > numel( p.Results.catSymbol ) 
    error('MATLAB:targplot', 'Not enough category labels/colours for number of columns in x,y arrays.' );
  end
  
  % convert x and y into cell array of classes for the plotting routine
  % this is not neede dwhen x and y are already cell arrays
  targ_x = cell( 1, size(x,2) );
  targ_y = cell( 1, size(x,2) );
  for ii=1:size(x,2)
    targ_x{ii} = x(:,ii);
    targ_y{ii} = y(:,ii);
  end
end

if isempty(findall(0,'Type','Figure'))
  % if no figure exists, create one...
  figure;
end

q = p.Results.targetMax;
filledCircle([0,0],1,1000, p.Results.Color );
hold on;
circle( 0, 0, 1, 'k-' );
circle( 0, 0, 0.5, 'k:' );
for k=1:numel(targ_x)
  h(k) = plot( targ_x{k}, targ_y{k}, p.Results.catSymbol{k} );
end
grid on;
if ~isempty( p.Results.catLabel )
  legend(h,p.Results.catLabel{1:numel(targ_x)});
end
xlim( [ -q q ] );
ylim( [ -q q ] );
line( [ -q q ], [ 0 0 ], 'Color', 'k', 'LineStyle', '--' );
line( [ 0 0 ], [ -q q ], 'Color', 'k', 'LineStyle', '--' );
xlabel( 'CRMSE/\sigma_O' );
ylabel( 'BIAS/\sigma_O' );
axis square;

text(  0.8*q, -0.8*q , 'SD', 'FontSize', 14, 'HorizontalAlignment', 'right' );
text( -0.8*q, -0.8*q,  'R', 'FontSize', 14, 'HorizontalAlignment', 'left' );

% little helper routine...
function [ tx, ty ] = target( o, m, oo, mm )

std_obs = std( oo );
avg_obs = mean( oo );
std_mod = std( mm );
avg_mod = mean( mm );

crmse  = sqrt( mean( ( ( m - avg_mod ) - ( o - avg_obs ) ).^2 ) ); % centered RMSE
bias   = avg_mod - avg_obs;

% calculate target plot sign
C     = corrcoef( m, o );
r     = C(1,2);
nmsd  = ( std_mod - std_obs ) ./ std_obs;

if r < 1
  f = nmsd ./ sqrt( 2.*(1-r));
  if f > 1,
    % SD dominates the error
    targ_sgn = 1;
  else
    % R dominates the error
    targ_sgn = -1;
  end
end

tx  = targ_sgn .* crmse ./ std_obs;
ty  = bias  ./ std_obs;

%x and y are the coordinates of the center of the circle
%r is the radius of the circle
%0.01 is the angle step, bigger values will draw the circle faster but
%you might notice imperfections (not very smooth)
function circle(x,y,r,varargin)

style = 'k-';
if nargin>3
  style=varargin{1};
end
ang=0:0.01:2*pi; 
xp=r*cos(ang);
yp=r*sin(ang);
plot(x+xp,y+yp,style);


function h = filledCircle(center,r,N,color)
%---------------------------------------------------------------------------------------------
% FILLEDCIRCLE Filled circle drawing
% 
% filledCircle(CENTER,R,N,COLOR) draws a circle filled with COLOR that 
% has CENTER as its center and R as its radius, by using N points on the 
% periphery.
%
% Usage Examples,
%
% filledCircle([1,3],3,1000,'b'); 
% filledCircle([2,4],2,1000,'r');
%
% Sadik Hava <sadik.hava@gmail.com>
% May, 2010
%
% Inspired by: circle.m [Author: Zhenhai Wang]
%---------------------------------------------------------------------------------------------

THETA=linspace(0,2*pi,N);
RHO=ones(1,N)*r;
[X,Y] = pol2cart(THETA,RHO);
X=X+center(1);
Y=Y+center(2);
h=fill(X,Y,color);
