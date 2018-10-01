% OPAQ_SIM Run the neural networks
%
%
% opaq_sim( cnf, pol_name, agg_str, start_date, end_date )
% opaq_sim( cnf, pol_name, agg_str, start_date, end_date, varargin )
% 
%
% Optional parameters :
%
%  'fc_hor', [0:4]        : array of forecast horizons to run, default is 
%                           given by 0 to cnf.fc_hor_max
%
%  'stations', { list }   : cell array of stations for which to run,
%                           default is all stations which contain the
%                           pollutant
%
%  'models', { list }     : cell array of model names to run, default is
%                           all
%
%  'mor_agg', <value>     : the hour at which the model is supposedly run
%                           default is 9, this cause. 
%
%  'output', <type>       : specify the output type, default is a simple
%                           matlab file with the xx_date, xx_pol and fc_pol
%                           values
%
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function opaq_sim( cnf, pol_name, agg_str, start_date, end_date, varargin )

% -- parse command line arguments
p = inputParser;
p.CaseSensitive = true;
p.addParameter( 'fc_hor', 0:cnf.fc_hor_max, @isnumeric );
p.addParameter( 'stations', {}, @iscell );
p.addParameter( 'models', opaq_getmodels, @(x) assert( iscell(x) && ~isempty(x) ) );
p.addParameter( 'mor_agg', 9, @isnumeric );
p.addParameter( 'output', 'matlab', @isstr );
p.parse( varargin{:} );

for k=1:length( p.Results.fc_hor )
    fc_hor = p.Results.fc_hor(k);

    % -- architecture dir for this forecast horizon
    arch_dir = opaq_strrep( cnf.io.archdir_pattern, ...
        'pol', pol_name,'mor_agg', p.Results.mor_agg, 'fc_hor', fc_hor );
    
    for i=1:length(cnf.network)
        s = cnf.network(i);
        
        % -- we have requested a particular station list, but this is not one of them...
        if ~isempty( p.Results.stations ) && ~ismember( s.name, p.Results.stations ), continue; end;
        
        % -- does the station measure the requested pollutant ?
        if ~any( strcmp( s.pollutants, pol_name ) ), continue; end;
        
        % -- create the samples for every model
        for j=1:length( p.Results.models )
            model_name = p.Results.models{j};
            
            try
            [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, s.name, ...
                pol_name, agg_str, p.Results.mor_agg, fc_hor, model_name, start_date, end_date );
            catch ME
                fprintf( '+++ skipping %s - %s : %s\n', station, model_name, ME.message );
                continue;
            end
            
            % do the output
            switch p.Results.output
                case ''                    
                    
                otherwise
                    % output_dir
                    output_dir = cnf.io.outdir_pattern;
                    if ~exist( output_dir, 'dir' ), mkdir( output_dir ); end;
                    % save as simple matlab file
                    fname = opaq_strrep( cnf.io.outfile_pattern, ...
                       'station', s.name, 'pol', pol_name, 'agg_str', agg_str, ...
                       'model', model_name, 'mor_agg', p.Results.mor_agg, 'fc_hor', fc_hor );
                    save( fullfile( output_dir, fname ), 'xx_date', 'xx_value', 'fc_value' );
            end
        end
        
    end

end