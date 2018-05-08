% forecast_models
%
% script to produce the forecasts
%
% Bino Maiheu

clear variables;
clear classes;
close all;


cnf = opaq_readconfig( 'etc/config.xml' );

opaq_forecast( cnf, 'pm10', 'dayavg', datenum(2014,3,1), datenum(2014,3,12), ...
    'mode', 'ann', 'models', { 'model1', 'model2', 'model3'}, ...
    'output', 'text' );
