%% LeavingOneOut
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)

clear variables
close all

%% General RIO configuration
pollutant  = 'pm10';
agg_time   = 'da';
gis_type   = 'clc06d';
grid_type  = '4x4';

weekpart   = 'all';
dates      = datenum(2000,01,01):datenum(0,0,1):datenum(2011,12,31);

% define in which setup file & configuration we're working
setup_file = 'rio_setup.xml';
setup_conf = 'lts';

%% RIO Inialisation
% configure rio
cnf = rio_setup( setup_file, setup_conf, pollutant, agg_time, gis_type, grid_type );
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

%% Here we go...
valid = struct();
valid.rmse = zeros( 1, cnf.nr_st );
valid.bias = zeros( 1, cnf.nr_st );
valid.r2   = zeros( 1, cnf.nr_st );

for st_id=1:cnf.nr_st
  have_station = true;
  
  % first check whether our station is not flagged in the xml file
  % normally data is get rid of in rio_dblookup,
  % need to change this at some point to make things a bit more
  % efficient...
  if isfield( cnf, 'stationList' )
    if ~any( find( strcmp( cnf.stationList, cnf.st_id{ st_id } ) ) )
      fprintf( '+++ EXPERIMENTAL: skipping %s based upon stationList in XML\n', cnf.st_id{ st_id } );
      have_station = false;
    else
      fprintf( '+++ EXPERIMENTAL: validating %s...\n', cnf.st_id{ st_id } );
    end
  end
    
  if have_station
    [ stats, mod, obs ] = rio_validate( cnf, st_id, dates );
  
    fprintf( '%s RMSE = %f\n', cnf.st_id{st_id}, stats.rmse );
    fprintf( '%s bias = %f\n', cnf.st_id{st_id}, stats.bias );
    fprintf( '%s R^2  = %f\n', cnf.st_id{st_id}, stats.r^2 );
  
    valid.rmse(st_id) = stats.rmse;
    valid.bias(st_id) = stats.bias;
    valid.r2(st_id)   = stats.r^2;

  else
    
  end
  

end

figure;
bar( valid.bias );
colormap summer;
