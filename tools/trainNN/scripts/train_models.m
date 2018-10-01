% train_models
%
%  Master script to train the OPAQ OVL models
%
%
% Bino Maiheu
% bino.maiheu@vito.be

close all;
clear variables;
clear classes;

addpath lib;

%% Configuration
start_date   = datenum( 2009, 1, 1);
end_date     = datenum( 2013,12,31);

cnf = opaq_readconfig( 'etc/config.xml' );

models = { 'model1', 'model2', 'model3' };

%% Model training
opaq_train( cnf, 'pm25', 'dayavg', start_date, end_date, 'models', models );

opaq_train( cnf, 'pm10', 'dayavg', start_date, end_date, 'models', models );
    
opaq_train( cnf, 'no2', 'dayavg', start_date, end_date, 'models', models );
opaq_train( cnf, 'no2', 'max1h', start_date, end_date, 'models', models );
    
opaq_train( cnf, 'o3', 'dayavg', start_date, end_date, 'models', models );
opaq_train( cnf, 'o3', 'max1h', start_date, end_date, 'models', models );
opaq_train( cnf, 'o3', 'max8h', start_date, end_date, 'models', models );

%% Now run the networks

% Optimizing PM25
opaq_optimize( cnf, 'pm25', 'dayavg', start_date, end_date, 'models', models );

% Optimizing PM10
opaq_optimize( cnf, 'pm10', 'dayavg', start_date, end_date, 'models', models );

% Optimizing NO2
opaq_optimize( cnf, 'no2', 'dayavg', start_date, end_date, 'models', models );
opaq_optimize( cnf, 'no2', 'max1h', start_date, end_date, 'models', models );

% Optimizing O3
opaq_optimize( cnf, 'o3', 'dayavg', start_date, end_date, 'models', models );
opaq_optimize( cnf, 'o3', 'max1h', start_date, end_date, 'models', models );
opaq_optimize( cnf, 'o3', 'max8h', start_date, end_date, 'models', models );
