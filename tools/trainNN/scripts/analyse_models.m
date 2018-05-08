
%% Configuration
%function analyse_models( pol_name, fc_hor )

st_network   = opaq_readstations( 'network.xml' );

pol_name     = 'o3';
model_name   = 'all_meteo';  % the all_meteo model enables one to do a correlation analysis
agg_str      = 'max8h';
mor_agg      = 9; % supposedly run the model at 9UTC 
fc_hor       = 0;

start_date   = datenum( 2009, 1, 1);
end_date     = datenum( 2014,12,31);

output_mode  = 0; % 0: no output, 1: new, 2: append
output_file  = sprintf( 'correlation_output2.txt' );


%% Initialize...
switch output_mode
    case 1
        fid = fopen( output_file, 'wt' );
        fprintf( fid, 'STATION\tPOLLUTANT\tMODEL\tAGGREGATION\tFCHORIZON\tVARIATE\tRHO\n' );
    case 2
        fid = fopen( output_file, 'at' );
    otherwise
end


% get some model information back
m = opaqmodels.( model_name );
vlist = m.input_names;
np    = length( vlist ); 
nrows = floor( sqrt( np ) );
ncols = ceil(np/nrows);


%% Loop over the staions...
for i=1:length(st_network)
    s = st_network(i);
    
    % -- does the station measure the requested pollutant ?
    if ~any( strcmp( s.pollutants, pol_name ) ),
        fprintf( '+++ station %s does not measure %s\n', s.name, pol_name );
        continue
    end    
    fprintf( 'processing %s\n', s.name );
    
    % -- create the samples
    [ dates, target, input ] = opaq_createsamples( s.name, ...
        pol_name, agg_str, mor_agg, fc_hor, model_name, start_date , end_date );

    if isempty(dates)
        warning( '+++ no data found for %s', s.name );
        continue
    end
    
    % -- output the training datasets
    
    % -- prepare a figure with the correlation plots
    figure( 'Position', [ 10 10 1400 800 ] );
    for k=1:np,
        subplot(nrows,ncols,k);
        set(gca, 'FontSize', 6 );
        plot( input(:,k), target, '.' );
        
        rho = corrcoef( input(:,k), target );
    
        % a title...
        xlabel( strrep( vlist{k}, '_', '\_' ), 'FontSize', 6 );
        title( sprintf( 'r = %.2f', rho(1,2) ), 'FontSize', 6 );
    
        % write out correlations
        if output_mode > 0
            fprintf( fid, '%s\t%s\t%s\t%s\t%d\t%d\t%s\t%f\n',s.name, pol_name, ...
                model_name, agg_str, mor_agg, fc_hor, vlist{k}, rho(1,2) );
        end
    end

    if output_mode > 0
        saveas( gcf, sprintf( 'opaq_correlation_analysis-%s-%s-%s-%s-%dUT-day%d', ...
            s.name, pol_name, agg_str, model_name, mor_agg, fc_hor ), 'png' );
        close(gcf);
    end

end



%% Finalize
if output_mode > 0 
    fclose(fid);
end