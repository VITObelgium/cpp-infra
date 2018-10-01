% validate_models
%
%  Master script to validate the different ANN models
%
%
% Bino Maiheu
% bino.maiheu@vito.be

close all;
clear variables;
clear classes;

addpath lib;
addpath 'D:\Matlab\MatlabToolkit\Validation';

%% Configuration

% -- define validation period (should differ from training period)
start_date   = datenum( 2014,  1,  1);
end_date     = datenum( 2014, 12, 31);

% -- select pollutants/aggregation
% sel = struct( 'polName',  { 'pm10', 'pm25', 'o3', 'o3', 'no2' }, ...
%     'aggrType', { 'dayavg', 'dayavg', 'max1h', 'max8h', 'max1h' } );
%sel = struct( 'polName',  { 'o3', 'o3', 'no2' }, ...
%    'aggrType', { 'max1h', 'max8h', 'max1h' } );

sel = struct( 'polName',  { 'pm25', 'pm10',  }, ...
     'aggrType', { 'dayavg', 'dayavg' } );

%sel = struct( 'polName',  { 'pm25' }, ...
%     'aggrType', { 'dayavg' } );

% -- models to include in the validation
models = { 'model1', 'model2', 'model3' };
%models = { 'model2' };

% -- read the configuration structure
cnf = opaq_readconfig( 'etc/config.xml' );

%% Loop over the requested pollutants
for k=1:numel(sel);    
            
    mkdir( fullfile( '.', 'valid', sel(k).polName, sel(k).aggrType, '') );
    
    opaq_forecast( cnf, sel(k).polName, sel(k).aggrType, start_date, end_date, ...
        'mode', 'ann', 'models', models, 'output', 'text', ...
        'valid', true, 'valid_indic', { 'rmse', 'bias', 'r2', 'nmb', 'rrmse', 'fcf', 'ffa' }, ...        
        'valid_file', sprintf( '%s_%s_valid_2014_rp99.txt', sel(k).polName, sel(k).aggrType ), ...
        'valid_plot', false, 'valid_plotdir', fullfile( '.', 'valid', sel(k).polName, sel(k).aggrType, '') );
    
end
