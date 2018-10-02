%% SpatialCorrelations
% The new version, based on libRIO
% (c) libRIO 2010/2011 (bino.maiheu@vito.be)


%% General RIO configuration
function BuildDatabase( pollutant )

% define in which setup file & configuration we're working
setup_file = 'rio_setup.xml';
setup_conf = 'v3.6'; % 'base';
driver     = 'clc06d'; % to pick up the correct station file...
%pollutant  = 'so2';

%% RIO Inialisation
% configure rio
cnf = rio_setup( setup_file, setup_conf, pollutant, 'da', driver, '4x4'  );
cnf.deployment = 'IRCEL';

% load station information
cnf = rio_loadstationinfo( cnf );

% create the database, no filename given, so program will ask !
rio_createdb( cnf )
