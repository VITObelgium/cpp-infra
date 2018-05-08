% OPAQ_TRAIN trains the neural network models
%
%
% opaq_train( cnf, pol_name, agg_str, start_date, end_date )
% opaq_train( cnf, pol_name, agg_str, start_date, end_date, varargin )
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
%  'output', <type>       : specify the output type, default is just to the
%                           terminal
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function opaq_train( cnf, pol_name, agg_str, start_date, end_date, varargin )


% -- parse command line arguments
p = inputParser;
p.CaseSensitive = true;
p.addParameter( 'fc_hor', 0:cnf.fc_hor_max, @isnumeric );
p.addParameter( 'stations', {}, @iscell );
p.addParameter( 'models', opaq_getmodels, @(x) assert( iscell(x) && ~isempty(x) ) );
p.addParameter( 'mor_agg', 9, @isnumeric );
p.parse( varargin{:} );

    
% -- loop over forecast horizons
for k=1:length( p.Results.fc_hor )
    fc_hor = p.Results.fc_hor(k);
    
    % -- architecture dir for this forecast horizon
    arch_dir = opaq_strrep( cnf.io.archdir_pattern, ...
        'pol', pol_name,'mor_agg', p.Results.mor_agg, 'fc_hor', fc_hor );
    if ~exist( arch_dir, 'dir' ), mkdir( arch_dir ); end;
    
    % -- create the samples for every model
    for j=1:length( p.Results.models )
        model_name = p.Results.models{j};
        
        % -- open training logfile
        fname = opaq_strrep( cnf.ann.logfile_pattern, ...
            'pol', pol_name, 'mor_agg', p.Results.mor_agg, 'model', model_name, 'fc_hor', fc_hor );
        fid = fopen( fullfile( arch_dir, fname ), 'wt' );
        if fid < 0,
            error( 'Cannot open logfile' );
        end
        
        fprintf( fid, '****************************************************************\n' );
        fprintf( fid, 'OPAQ training log \n' );
        fprintf( fid, '\n' );
        fprintf( fid, ' - polluent                : %s\n', pol_name );
        fprintf( fid, ' - aggregation             : %s\n', agg_str );
        fprintf( fid, ' - forecast horizon        : day%d\n', fc_hor );
        fprintf( fid, ' - operational run start   : %dUT\n', p.Results.mor_agg );
        fprintf( fid, ' - model_name              : %s\n', model_name );
        fprintf( fid, ' - training dates start    : %s\n', datestr( start_date ) );
        fprintf( fid, ' - training dates end      : %s\n', datestr( end_date ) );
        fprintf( fid, '****************************************************************\n' );       
        
        for i=1:length(cnf.network)
            s = cnf.network(i);
            
            fprintf( fid, 'training %s', s.name );
            
            % -- does the station measure the requested pollutant ?
            if ~any( strcmp( s.pollutants, pol_name ) ),
                continue
            end

            % -- effectively do the training... 
            try                
                [ n_samples ] = opaq_trainnn( cnf, s.name, pol_name, agg_str, p.Results.mor_agg, fc_hor, ...
                    model_name, start_date, end_date );
               
            catch ME
                if strcmp( ME.identifier, 'OPAQ:DataNotFound' )                    
                    fprintf( fid, 'no data found... \n' );
                    continue;
                else 
                    rethrow(ME);
                end                                   
            end
                        
            
        end
        
        % -- close log file...
        fclose(fid);
    end
    
end


