% OPAQFC OPAQ Real time foreacst 
%   
%   opaqfc( cnf, pol_name, agg_str, start_date, end_date )
%   opaqfc( cnf, pol_name, agg_str, start_date, end_date, varargin )
%
% similar as the opas_foreacst routine, however the start_date & end_dates
% are now interpreted as base dates, so this one can be run in real time. No
% validation options are present here and the output is simply ascii files
% per day as with the operational version simpleAsciiOutputWriter plugin. 
%
%
% Available options are :  
% 
%  'fc_hor', [0:4]           : array of forecast horizons to run, default is
%                              given by 0 to cnf.fc_hor_max
%
%  'stations', { list }      : cell array of stations for which to run,
%                              default is all stations which contain the
%                              pollutant
%
%  'mor_agg', <value>        : the hour at which the model is supposedly run
%                              default is 9, this cause.
%
%  'models', { ... }         : run the forecast for these models, only for
%                              ann and rtc modes
%
% See also opaq_forecast
%
% Author: Bino Maiheu
% Contact : bino.maiheu@vito.be

function opaqfc( cnf, pol_name, agg_str, start_date, end_date, varargin )

% -- parse command line arguments
p = inputParser;
p.CaseSensitive = true;
p.addParameter( 'fc_hor', 0:cnf.fc_hor_max, @isnumeric );
p.addParameter( 'stations', {}, @iscell );
p.addParameter( 'mor_agg', 9, @isnumeric );
p.addParameter( 'models', opaq_getmodels, @(x) assert( iscell(x) && ~isempty(x) ) );

% -- parse the command line arguments
p.parse( varargin{:} );

for dd = start_date:datenum(0,0,1):end_date
    
    fprintf( 'Basetime %s\n', datestr(dd) );
    
    fid = fopen( sprintf( 'opaqfc_%s_%s_%s.txt', pol_name, agg_str, datestr(dd,'yyyy-mm-dd') ), 'wt' );
    if fid<0
        error( 'cannot open file ' );
    end
    
    % -- looping over the stations
    for i=1:length(cnf.network)
        s = cnf.network(i);
        
        % -- we have requested a particular station list, but this is not one of them...
        if ~isempty( p.Results.stations ) && ~ismember( s.name, p.Results.stations ), continue; end;
        
        % -- does the station measure the requested pollutant ?
        if ~any( strcmp( s.pollutants, pol_name ) ), continue; end;
        
        
        fprintf( ' -- forecasting %s...\n', s.name );
        
        % -- loop over the forecast horizons
        for k=1:length( p.Results.fc_hor )
            fc_hor = p.Results.fc_hor(k);
            
            % --------------------------------------
            % adjust time window for the forecast...
            % --------------------------------------
            fc_date = dd + fc_hor;
           
            fprintf( fid, '%s\t%s\t%s', datestr(dd,'yyyy-mm-dd'), s.name, datestr(fc_date,'yyyy-mm-dd') );
            
            % ------------------------------
            % loop over all the given models
            % ------------------------------
            for j=1:length( p.Results.models )
                model_name = p.Results.models{j};
                
                try
                    % produce the ann forecast for this model & fc horizon
                    [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, s.name, pol_name, ...
                        agg_str, p.Results.mor_agg, fc_hor, model_name, fc_date, fc_date );
                    
                    fprintf( fid, '\t%f', fc_value );
                    
                catch ME
                    switch ME.identifier
                        case 'OPAQ:DataNotFound'
                            fprintf( fid, '\t-9999' );
                            continue; % skipping
                        otherwise
                            rethrow(ME);
                    end
                end
                
                    
            end % loop over models
            
            fprintf( fid, '\n' );
            
        end % loop over forecast horizons
        
        
    end % loop over stations
    
    
    fclose(fid);
    
end % loop over basetimes
