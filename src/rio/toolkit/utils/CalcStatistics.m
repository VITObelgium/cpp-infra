%% CalcStatistics
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)

%clear all
%close all

%% General RIO configuration
pollutant  = 'o3';
agg_time   = 'da';
gis_type   = 'clc06d';
grid_type  = '4x4';

weekpart   = 'all';
window     = [ datenum( 2008, 01, 01, 0, 0, 0 ) datenum( 2012, 12, 31, 23, 59, 0 ) ];

% define in which setup file & configuration we're working
setup_file = 'rio_setup.xml';
setup_conf = 'v3.6';

%% RIO Inialisation
% configure rio
cnf = rio_setup( setup_file, setup_conf, pollutant, agg_time, gis_type, grid_type  );
cnf.deployment = 'IRCEL';

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

%% Trend determination options, see help rio_gettrend for more info
options = struct( ...    
    'weekpart',    weekpart, ...         % part of the week
    'time_window', window, ...           % window to compute statistics
    'cutoff',      500, ...              % values above the cutoff concentration are ignored in the statistics
    'overwrite',  'ask' );               % overwrite the trend parameters

%-- we determine the trend
[ xx_avg, xx_std ] = rio_calcstats( cnf, options );

