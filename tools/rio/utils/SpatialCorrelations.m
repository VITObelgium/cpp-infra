%% SpatialCorrelations
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)


% clear all
% close all

%% General RIO configuration
pollutant  = 'so2';
agg_time   = 'da';
gis_type   = 'clc06d';
grid_type  = '4x4';

window     = [ datenum( 2001, 01, 01, 0, 0, 0 ) datenum( 2011, 12, 31, 23, 59, 0 ) ];

% define in which setup file & configuration we're working
setup_file = 'rio_setup.xml';
setup_conf = 'ltslog_optim';

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
end

%% Setup the option structure
sp_options = struct( ...
    'time_window', window, ...  % use this time window
    'make_plot', true, ...      % make a nice plot
    'short', false, ...          % use short range correlation model as well
    'short_range', 20., ...     % the range for it
    'detrend', true, ...        % detrend the data
    'profile', true, ...       % fit the profile histogram    
    'prof_binsize', 20., ...    % binsize in km for profile histogram
    'corr_model', 'exp', ...    % spatial correlation model (exp, expm)
    'overwrite', 'ask' );

%-- we determine the spatial correlation model
[ p, p_short ] = rio_calcspcorr( cnf, sp_options );
