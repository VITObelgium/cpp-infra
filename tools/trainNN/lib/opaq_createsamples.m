% OPAQ_CREATESAMPLES
%
% Creates a set of samples for a station, pollutant, aggregation time
% given a specific model configuration. The start & stop date
% are to select the training period
%
%
% [ dates, target, input, errcode ] = opaq_createsamples( ...
%    cnf, station, pollutant, aggregation, mor_agg, fc_hor, model_name, ...
%    start_date, stop_date )
%
% Bino Maiheu (c) 2015 VITO

function [ dates, target, input, errcode ] = opaq_createsamples( ...
    cnf, station, pollutant, aggregation, mor_agg, fc_hor, model_name, ...
    start_date, stop_date )

% -- start with empty hands...
dates   = [];
target  = [];
input   = [];
errcode = 0; 

% -- loading xx_meteo, xx_obs and xx_info
load( fullfile( cnf.io.data_path, sprintf( '%s.mat', station ) ) );
load( fullfile( cnf.io.meteo_path, sprintf( 'METEO_%s.mat', xx_info.meteo ) ) );

% -- generate target values
if ~isfield( xx_obs, pollutant)
    errcode = 1; % station does not contain pollutant
    return; 
end
if ~isfield( xx_obs.( pollutant ),  sprintf( 'xx_%s', aggregation ) )
    errcode = 2; % no such aggregation
    return;
end

% -- get the dates & target
dates  = xx_obs.( pollutant ).xx_days;
target = xx_obs.( pollutant ).( sprintf( 'xx_%s', aggregation ) );

% -- now the input values... depends on the model
m = opaqmodels.( model_name )( pollutant, aggregation, mor_agg, xx_obs.(pollutant), xx_meteo );
dates  = [];
target = [];
input  = [];

for fc_date=start_date:datenum(0,0,1):stop_date
    [ target_sample, input_sample, have_sample ] = m.make_sample( fc_date, fc_hor );
    if ( have_sample )
       target = [ target; target_sample ];
       input  = [ input; input_sample ];
       dates  = [ dates; fc_date ];
    end
end


% -- handle missing values 


end


