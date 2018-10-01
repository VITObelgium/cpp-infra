% VALIDSTATS Get model/observation validation statistics
%
% This routine provides some extended valudation information for model/
% observation pairs. The routine can also handle timeseries with different 
% domains and intersects them or interpolates them (see options). 
%
% Description
% 
% s = validstats( obs, mod )
% s = validstats( obs, mod, 'param', value, ... )
% [ s, hFig ] = validstats( obs, mod, 'makePlot', true, 'param', value, ... )
%
% The sizes of obs and mod can either be a vector in which case they
% have to be of the same size, or a Nx2D array, in which case the first
% column is assumed to the the date or some index. 2xND matrices are first 
% transposed to Nx2D. 
% 
% When the size of obs and mod is Nx2D instead of Nx1D, the first column
% is assumed to be the date (or some other unique index e.g. when doing spatial
% validation), meaning, it's value is used to intersect the model and
% observation values to matching dates. When it is a continuous variable
% such as a timestamp, and not an index, one can use the interpolTime
% option to linearly interpolate the model values to the obervatione before
% validating. 
%
%
% Available validation parameters
%
% 'exceedThresh', <val> ..... : define exceedance treshold, calculate 
%                               exceedance statististics, default NaN (not
%                               taken into account)
%
% 'exceedTol', <val> ........ : exceedance tolerance, default 0. 
%
% 'missingValue', <val> ..... : alternative missing value other than NaN.
%
% 'rpeValue', <val> ......... : rel. percentile error value, this is the
%                               percentile value at which the rel. error
%                               is evaluated (unmatched). e.g. for the 35
%                               days limit for pm10 daily averages : 
%                               p = 100.*(365-35)/365;
%
% 'interpolTime', <t/f> ..... : instead of intersecting the time arrays in 
%                               obs/mod matrices, we interpolate linearly 
%                               the model times to the observations to
%                               match.
%
% 'ksTest', <t/f> ........... : do a Kolmogorov-Smirnov test for equality of
%                               distributions of model and observed values
%
% 'robustFit', <t/f> ........ : do model vs obs regrssion using robustfit
%
% Available plotting parameters
%
% 'makePlot', <t/f> ......... : make validation plots and export them to
%                               the optional output argument (struct).
% 
% 'obsLabel', <'name'> ...... : label for the observations (plot label)
% 'modLabel', <'name'> ...... : label for the model vals (plot label)
%
% 'dateTicks', <t/f> ........ : add dateticks to the plot assuming the 
%                               index arrays are timestamps (datenum)
%
% 'plotTitle', <'name'> ..... : a master title for the plot.
% 'targetMax', <val> ........ : maximum for target plot, def. 2.5
% 
% 'storeData', <t/f> ........ : the stripped o/m timeseries are stored in
%                               the output structure as s.o and s.m
%
% Example
%
%  xx_date = datenum(2010,1,1):datenum(0,0,1):datenum(2010,12,31);
%  obs = [ xx_date', 100.*rand(size(xx_date'))];
%  mod = [ xx_date', 100.*rand(size(xx_date'))];
%  s = validstats( mod, obs, 'missingValue', -9999., ...
%                  'exceedThresh', 50., 'rpeValue', 100.*(365-35)/365, ...
%                  'makePlot', true );
%
% Results
%
% Calculated statistics in the structure are : 
%
% - s.rmse      Root Mean Squared error
% - s.bias      Bias mod-obs
% - s.absbias   Absolute value of bias mod-obs
% - s.r         Pearson correlation coefficient
% - s.r2        Explained variance
% - s.std_obs   Observations sample standard deviation (N-1 norm)
% - s.avg_obs   Observations sample arverage
% - s.std_mod   Model sample standard deviation (N-1 norm)
% - s.avg_mod   Model sample arverage
% - s.crmse     Centered RMSE
% - s.rrmse     Relative RMSE (w.r.t. avg_obs)
% - s.nmb       Normalised Mean Bias
% - s.mfb       Mean Fractional Bias
% - s.mfe       Mean Fractional Error
% - s.nmsd      Normalised mean standarddeviation
% - s.target    Target Value
% - s.targ_x    Target plot x value (normalised CRMSE)
% - s.targ_y    Target plot y value (normalised bias)
% - s.ioa       Index of Agreement
% - s.fac2      Fraction of model values withing a fraction of 2 of the obs
% - s.noe_mod   Number of exceedances in the model
% - s.noe_obs   Number of observed exceedances
% - s.si        Successindex
% - s.fcf       Fraction of correct forecast events
% - s.frf       Fraction of realised forecast events
% - s.sfn       Skill of forecasting non-events
% - s.tsc       Threat score
% - s.ffa       Fraction of false alarm (good definition)
% - s.fi        Failure Index
% - s.rpe       Relative percentile error
% - s.kstest    Kolmogorov Smirnov test statistics (structure)
%
% In case the user wants plots to be generated, they are provided in the 
% second, optional output structure array. 
%
% Notes
% - The target plot values are normalised with the standard deviation of
%   the observations (cfr. first version of target plot), the sign of the
%   x-axis value (CRMSE) is determined howevera according to the later
%   versions of the target plot, i.e. 
% 
% 
% Todo
%  - add AOT40 calculation and comparison
%
%
% Reference
% - Thunis, P., Georgieva E., Pederzoli A., The DELTA tool and Benchmarking
%   Report - Concepts and Users' Guide - version 1, JRC, 2011. 
%
% - Thunis, P., Pederzoli, A. and Pernigotti, D.: Performance criteria to 
%   evaluate air quality modeling applications, Atmospheric Environment, 59,
%   476–482, doi:10.1016/j.atmosenv.2012.05.043 
%
% See also validprepro, targplot, kstest2
%
% Bino Maiheu (bino.maiheu@vito.be)
% Copyright 2012-2014, Flemish Instititute for Technological Research

% History
% * [BM, 2013-08-23] - first version 

% TODO
% - add some functionality to aggregate values to a common timebase somehow

function [ s, varargout ] = validstats( obs, mod, varargin )

% Parse input
p = inputParser;
p.CaseSensitive = true;
p.addParamValue( 'rpeValue', NaN, @isnumeric );
p.addParamValue( 'exceedThresh', NaN, @isnumeric );
p.addParamValue( 'exceedTol',      0, @isnumeric );
p.addParamValue( 'missingValue', NaN, @isnumeric );
p.addParamValue( 'interpolTime', false, @islogical );
p.addParamValue( 'ksTest',       false, @islogical );
p.addParamValue( 'robustFit',    false, @islogical );
p.addParamValue( 'makePlot',     false, @islogical );
p.addParamValue( 'obsLabel',     'obs', @isstr );
p.addParamValue( 'modLabel',     'mod', @isstr );
p.addParamValue( 'dateTicks', false, @islogical );
p.addParamValue( 'plotTitle', 'Validation', @isstr );
p.addParamValue( 'targetMax', 2.5, @isnumeric );
p.addParamValue( 'storeData', false, @islogical );
p.parse( varargin{:} );


%% Preprocess the input arrays for validation
[ have_index, o, m, oo, mm ] = validprepro( mod, obs, ...
  'missingValue', p.Results.missingValue, ...
  'interpolTime', p.Results.interpolTime );

% for plotting, convert the missing values to NaN
if have_index    
  if size(obs,1) == 2, obs = obs'; end;
  if size(mod,1) == 2, mod = mod'; end;
  
  obs( obs(:,2) == p.Results.missingValue, 2 ) = NaN;
  mod( mod(:,2) == p.Results.missingValue, 2 ) = NaN;
else 
  obs = obs(:);
  mod = mod(:);
  
  obs( obs == p.Results.missingValue ) = NaN;
  mod( mod == p.Results.missingValue ) = NaN;
end
n = length(o);

%% And now compute the validation statistics
C      = corrcoef( m, o );
s      = struct();
if p.Results.storeData
  s.o    = o;
  s.m    = m;
end
  
s.n    = length(o);
s.rmse = sqrt( mean( ( m - o ).^2 ) );
s.bias = mean( m - o );
s.absbias = abs( s.bias );
s.r2   = C(1,2).*C(2,1);
s.r    = C(1,2);

% compute some separate statistics


s.std_obs = std( oo ); % normalised to N-1, unmatched
s.avg_obs = mean( oo );
s.std_mod = std( mm );
s.avg_mod = mean( mm );

% some more exotic statistics such as CRMSE and Target value
s.crmse  = sqrt( mean( ( ( m - s.avg_mod ) - ( o - s.avg_obs ) ).^2 ) ); % centered RMSE
s.rrmse  = s.rmse ./ s.avg_obs; % relative RMSE
s.nmb    = s.avg_mod ./ s.avg_obs - 1; % normalised mean bias
s.mfb    = mean( 2.*( m - o )./( m + o ) );
s.mfe    = mean( 2.* abs( m - o )./ ( m + o ) );
s.nmsd   = ( s.std_mod - s.std_obs ) ./ s.std_obs;
s.target = sqrt( ( s.bias ./ s.std_obs ).^2 + ( s.crmse ./ s.std_obs ).^2 );
s.targ_y = s.bias  ./ s.std_obs;
s.targ_x = s.crmse ./ s.std_obs;

% calculate target plot sign
s.targ_sgn = 1;
if s.r < 1
  f = s.nmsd ./ sqrt( 2.*(1-s.r));
  if f > 1, 
    % SD dominates the error
    s.targ_sgn = 1;
  else
    % R dominates the error
    s.targ_sgn = -1;
  end
end
s.targ_x = s.targ_sgn * s.targ_x;

s.ioa    = 1 - s.n .* s.rmse.^2 ./ sum( ( abs(m-s.avg_obs)+(o-s.avg_obs) ).^2 ); % index of agreement
s.fac2   = numel( 0.5 <= abs(m./o) & abs(m./o) <= 2 ) ./ s.n; 

% do linear regression (use robustfit)
if p.Results.robustFit
  b = robustfit(o,m);
  s.slope = b(2);
  s.icept = b(1);
else 
  b = polyfit(o,m,1);
  s.slope = b(1);
  s.icept = b(2); 
end
 
% do thresold scores
if ~isnan( p.Results.exceedThresh )
  s.noe_treshold = p.Results.exceedThresh;
  s.noe_mod = length( find( m > p.Results.exceedThresh ) );
  s.noe_obs = length( find( o > p.Results.exceedThresh ) );
  
  sigmov = zeros( 1, n );
  sigmog = zeros( 1, n );
  for i=1:n
    sigmov(i) = sigmo( m(i), p.Results.exceedThresh, p.Results.exceedTol );
    sigmog(i) = sigmo( o(i), p.Results.exceedThresh, p.Results.exceedTol );
  end
  
  % Some explanatory doodle : 
  %            thresh
  %              v
  %   |----------|----------|
  %   |          |          |
  % v |    gV    |    GV    |
  % o |          |          |
  % o |----------|----------| <- threshold
  % r |          |          |
  % s |    gv    |    Gv    |
  % p |          |          |
  %   |----------|----------|
  %       geobserveerd
  
  GV  = sigmog*sigmov';
  gV  = (1-sigmog)*sigmov';
  Gv  = sigmog*(1-sigmov)';
  gv  = (1-sigmog)*(1-sigmov)';
  
  px = GV + gV; % total number of predicted exceedances (V=Voorspeld, G=Geobserveerd)
  ox = GV + Gv; % total number of observed exceedances
  a  = GV;      % total number of observed an predicted exceedances
  N  = GV + gV + Gv + gv; % grand total
  
  s.si  = 100*(a/ox+(N-(ox+px-a))/(N-ox)-1); % successindex
  s.fcf = 100*a/ox;                          % fraction of correct forecast events
  s.frf = 100*a/px;                          % fraction of realised forecast events
  s.sfn = 100*(N-(ox+px-a))/(N-ox);          % skill of forecasting non-events
  s.tsc = 100*a/(ox+px-a);                   % Threat score
  s.ffa = 100*(px-a)/px;                     % Fraction of false alarm (good definition)
  s.fi  = 100*((px-a)/ox - a/ox);            % Failure Index (new)
end

% do percentile statistics
if ~isnan( p.Results.rpeValue )
  s.rpe = abs( prctile( oo, p.Results.rpeValue ) - prctile( mm, p.Results.rpeValue ) ) ...
    ./ prctile( oo, p.Results.rpeValue );  
end

if p.Results.ksTest
  [ks_h, ks_p, ks_k ] = kstest2( oo, mm );
  s.kstest = struct( 'different', ks_h, 'pvalue', ks_p, 'statistic', ks_k );
end


if p.Results.makePlot
  set(0, 'DefaultAxesFontName', 'Calibri' );
  scrsz = get(0,'ScreenSize');
  hFig  = figure( 'Position',[scrsz(3)/8 scrsz(4)/8 6*scrsz(3)/8 6*scrsz(4)/8] );
  
  % config: plot limits
  qqlim = 1.1*max( [ oo; mm ] );
  ppmin = 0.9*min( [ oo; mm ] );
  ppmax = 1.1*max( [ oo; mm ] );
  dpp   = ppmax-ppmin;
  
  % -- do scatter plot
  subplot( 2, 3, 1 );
  plot( o, m, '.' );
  xlabel( p.Results.obsLabel );
  ylabel( p.Results.modLabel );
  grid on;
  xlim( [ ppmin ppmax ] );
  ylim( [ ppmin ppmax ] );
  title( 'Scatter mod vs. obs' );
  hold on;
  line( [ ppmin ppmax ], [ ppmin ppmax ], 'Color', 'r' );
  line( [ ppmin ppmax], [ ppmin ppmax/2 ], 'Color', 'k', 'LineStyle', '--' );
  line( [ ppmin ppmax], [ ppmin 2.*ppmax ], 'Color', 'k', 'LineStyle', '--' );
  plot( ppmin:ppmax, polyval( [ s.slope s.icept ], ppmin:ppmax ), 'g-' );
  % add some text
  text( ppmin+0.05*dpp, ppmax-0.05*dpp, { ...
    sprintf( 'RMSE \t= %.2f', s.rmse ); ...
    sprintf( 'BIAS \t= %.2f', s.bias ); ...
    sprintf( 'R^2  \t= %.2f', s.r2 ) }, ...
    'FontName', 'Calibri', 'FontSize', 9, 'BackgroundColor', [ 1. .8 .8 ], ...
    'LineStyle', '-', 'LineWidth', 1, 'EdgeColor', 'k', ...
    'HorizontalAlignment', 'left', 'VerticalAlignment', 'top' );
    
  % -- do target plot
  subplot( 2, 3, 2 );
  targplot( s.targ_x, s.targ_y, 'inputMode', 'coords', ...
                                'targetMax', p.Results.targetMax, ...
                                'catSymbol', { 'rs' }, 'catLabel', { p.Results.modLabel } );
  title( 'Target plot (v1)' );
  
  % -- do QQ plot
  subplot( 2, 3, 3 );
  qqplot( oo, mm );
  xlabel( 'Obs. quantiles' );
  ylabel( 'Mod. quantiles' );  
  xlim( [ 0 qqlim ] ); ylim( [ 0 qqlim ] );
  line( [ 0 qqlim ], [ 0 qqlim ] , 'Color', 'k');
  grid on;
  legend( p.Results.modLabel );
  title( 'QQ plot' );  
  box on;
    
  % -- do timeseries/index plot
  subplot( 2, 3, [ 4 5 6 ] );
  if have_index, plot( obs(:,1), obs(:,2), 'k' ); else plot( obs, 'k' ); end;
  hold on;
  if have_index, plot( mod(:,1), mod(:,2), 'r' ); else plot( mod, 'r' ); end;
  if ~isnan( p.Results.exceedThresh )
    if have_index
      range = [ min(obs(:,1)) max(obs(:,1)) ];      
    else
      range = [ 1 length(obs) ];
    end
    line( range, [ p.Results.exceedThresh p.Results.exceedThresh ], ...
      'Color', 'k', 'Linestyle', '--' );    
  end
  hold off;  
  grid on;
  legend( p.Results.obsLabel, p.Results.modLabel );
  
  if p.Results.dateTicks
    datetick;
  end
  
  % now put master title in there
  axes('Position',[0 0 1 1],'Xlim',[0 1],'Ylim',[0 1],'Box', 'off', ...
    'Visible','off','Units','normalized', 'clipping' , 'off');
  text( 0.5, 0.99, p.Results.plotTitle, ...
    'HorizontalAlignment','center', 'VerticalAlignment', 'top', ...
    'FontSize', 12, 'FontName', 'Calibri' );  
  
  
  % output figure ?
  if nargout == 2
    varargout{1} = hFig;
  end
  
else
  varargout{1} = {};
end


function v_out = sigmo( v_in, limit, tol )
if tol==0
    v_out( v_in < limit ) = 0;  % original heavyside
    v_out( v_in>=limit  ) = 1;
else
    v_out=(tanh((v_in - limit)/tol)/tanh(limit/tol) + 1)/2;
end
