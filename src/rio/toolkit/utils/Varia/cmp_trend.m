close all;
clear variables;

% compare trends
%% General RIO configuration
pollutant  = 'pm10';
agg_time   = 'da';
gis_type   = 'clc06d';
grid_type  = '4x4';
weekpart   = 'all';

% define in which setup file & configuration we're working
setup_file = 'rio_setup.xml';
setup_conf = 'lts';

%% RIO Inialisation
% configure rio
cnf = rio_setup( setup_file, setup_conf, pollutant, agg_time, gis_type, grid_type  );
cnf.deployment = 'IRCEL';

% load station information
cnf = rio_loadstationinfo( cnf );

%% Trend determination options, see help rio_gettrend for more info
trend_options = struct( ...
    'weekpart',        weekpart, ...      % part of the week 
    'edgefactor',      0.1, ...
    'fitmode',         'polyfit', ...
    'indic_step',      0.001, ...
    'save_plot',       false, ...          % save the plot
    'show_plot',       true, ...          % show the plot
    'show_stats',      true, ...          % some regression statistics
    'plot_labels',     false, ...         % plot the station labels as well
    'label_dx',        0.005, ...         % label horizontal shift
    'label_dy',        0.5,  ...          % label vertical shift
    'fancy_plot',      true, ...          % different colour per station type
    'fancy_legend',    true, ...          % legend for fancy plot
    'plot_avg_yrange', [ 0 60 ], ...      % fix y range for avg trend plot
    'plot_std_yrange', [ 0 30 ] );

rio_plottrend( cnf, trend_options );

%% compar with : 
my_paramPath = sprintf( './param/v3.4/%s', pollutant );
%gis_type = 'CorineID';
fname = fullfile( my_paramPath, 'trend', ...
            sprintf( 'avg_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            gis_type, weekpart, agg_time ) );
fprintf( 'Loading %s...\n', fname );
tmp = load( fname, 'p_avg');
p_avg = tmp.p_avg;
subplot( 1, 2, 1 ); 
hold on;
b=0:0.05:1.5;
plot( b, polyval( p_avg, b ), 'k:' );

tmp = load( fullfile( my_paramPath, 'trend', ...
            sprintf( 'std_trend_%s_%s_%s_agg_time-%s.mat',  cnf.pol_xx, ...
            gis_type, weekpart, agg_time ) ), 'p_std');
p_std = tmp.p_std;
subplot( 1, 2, 2 ); 
hold on;
b=0:0.05:1.5;
plot( b, polyval( p_std, b ), 'k:' );
       
