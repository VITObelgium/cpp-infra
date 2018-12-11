%% TrendWatcher
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)

%% General RIO configuration
pollutant  = 'so2';
agg_time   = 'da';
gis_type   = 'clc06d';
grid_type  = '4x4';

weekpart   = 'weekend';

% define in which setup file & configuration we're working
setup_file = 'rio_setup.xml';
setup_conf = 'ltslog_optim';

% plot ranges (in std. concentration units (log trans is automatic)
avg_yrange = [ 0 80 ];
std_yrange = [ 0 30 ];

%% RIO Inialisation
% configure rio
cnf = rio_setup( setup_file, setup_conf, pollutant, agg_time, gis_type, grid_type  );
cnf.deployment = 'IRCEL';
cnf.ipol_mode  = 'RIO';

% load station information
cnf = rio_loadstationinfo( cnf );

% load the measurements database
cnf = rio_loaddb( cnf );

% Apply data quality checks, e.g. drop periods with high PM10 for IRCEL or
% drop winter data for o3s, this routine internally checks what deployment
% you're working in, as data quality checks can differ for different 
% implementations
cnf = rio_dbdaql( cnf );

% apply the log transformation outside of the load/create database routines
% also best do this after the data quality checks...
if cnf.Option.logtrans
  fprintf( 'Applying log transformation to measurement database...\n' );
  cnf.xx_val(:,2:end) = log( 1. + cnf.xx_val(:,2:end) );
  avg_yrange = log( 1 + avg_yrange );
  std_yrange = log( 1 + std_yrange );
end

%% Trend determination options, see help rio_gettrend for more info
trend_options = struct( ...
    'calcstats',       false, ...         % re-compute statistics, or load ?
    'weekpart',        weekpart, ...      % part of the week 
    'fitmode',         'nlinfit', ...     % polyfit / nlinfit
    'ignore_stations', [ ], ...           % list of stations not to include in trend fitting
    'make_plot',       true, ...          % make the plot
    'save_plot',       true, ...          % save the plot
    'show_plot',       true, ...          % show the plot
    'show_stats',      true, ...          % some regression statistics
    'plot_labels',     false, ...         % plot the station labels as well
    'label_dx',        0.005, ...         % label horizontal shift
    'label_dy',        0.5,  ...          % label vertical shift
    'fancy_plot',      true, ...          % different colour per station type
    'fancy_legend',    true, ...          % legend for fancy plot
    'plot_avg_yrange', avg_yrange, ...    % fix y range for avg trend plot
    'plot_std_yrange', std_yrange, ...    % fix y range for std trend plot
    'overwrite',       'ask' );           % overwrite the trend parameters

%-- we determine the trend
rio_gettrend( cnf, trend_options );

