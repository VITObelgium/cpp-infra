% OPAQ_SIMNN Workhorse routine for running the nn
%
% [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, ...
%     station, pol_name, agg_str, mor_agg, fc_hor, model_name, ...
%     start_date, end_date )
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, ...
    station, pol_name, agg_str, mor_agg, fc_hor, model_name, ...
    start_date, end_date )


% -- construct the input samples
[ dates, target, input ] = opaq_createsamples( cnf, station, ...
    pol_name, agg_str, mor_agg, fc_hor, model_name, start_date, end_date );
            
if isempty(dates)    
    error( 'OPAQ:DataNotFound', 'no data found' );    
end
            
fprintf( 'opaq_simnn: %s %s %s day%d %02dUT %s\n', ...
    station, pol_name, agg_str, fc_hor, mor_agg, model_name );
            
% import the neural network
arch_dir = opaq_strrep( cnf.io.archdir_pattern, ...
    'pol', pol_name,'mor_agg', mor_agg, 'fc_hor', fc_hor );

fname = opaq_strrep( cnf.io.netfile_pattern, ...
    'station', station, 'pol', pol_name, 'agg_str', agg_str, ...
    'model', model_name, 'mor_agg', mor_agg, 'fc_hor', fc_hor );
if ~exist( fullfile( arch_dir, fname ), 'file' )
    error( 'OPAQ:ArchitectureNotFound', 'architecture does not exist' );
end
                
% -- import the networks
[ net, input_PS, target_PS ] = opaq_readffnet( fullfile( arch_dir, fname ) );
                        
% -- apply the normalisation to the input vector
input_n  = opaq_mapstd( 'apply', input', input_PS );
            
% -- run the neural network
output_n = sim(net, input_n);
            
% -- reverse the noramalization on the output
output   = opaq_mapstd( 'reverse', output_n, target_PS );
            
% -- reverse the log transform, also for the target values
xx_date  = dates;
xx_value = exp( target ) - 1;
fc_value = exp( output' ) - 1;
