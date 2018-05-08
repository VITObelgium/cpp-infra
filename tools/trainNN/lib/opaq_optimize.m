% OPAQ_OPTIMIZE Compute a tune for each station
%
% opaq_optimize( cnf, pol, agg_str, varargin )
%
% Available parameters
%  'fc_hor', [0:4]        : array of forecast horizons to run, default is 
%                           given by 0 to cnf.fc_hor_max
%
%  'stations', { list }   : cell array of stations for which to run,
%                           default is all stations which contain the
%                           pollutant
%
%  'models', { list }     : cell array of model names to choose from, default
%                           is all
%
%  'mor_agg', <value>     : the hour at which the model is supposedly run
%                           default is 9, this cause. 
%
%  'rtc_modes', [0:2]     : give a list of RTC modes to scan, default 0:2
%
%  'rtc_param', [0:20]    : give a list of RTC parameters to scan for rtc
%                           modes which use a parameter value, default
%                           0:20
%
%  'tune_file', <name>    : speficy the name of the output XML tune file
%
% Author  : Bino Maiheu, (c) 2015 VITO
% Contact : bino.maiheu@vito.be

function opaq_optimize( cnf, pol_name, agg_str, start_date, end_date, varargin )


% -- parse command line arguments
p = inputParser;
p.CaseSensitive = true;
p.addParameter( 'fc_hor', 0:cnf.fc_hor_max, @isnumeric );
p.addParameter( 'stations', {}, @iscell );
p.addParameter( 'models', opaq_getmodels, @(x) assert( iscell(x) && ~isempty(x) ) );
p.addParameter( 'mor_agg', 9, @isnumeric );
p.addParameter( 'tune_file', sprintf( 'opaq_tune_%s_%s-%s-%s.xml', ...
    pol_name, agg_str, datestr( start_date, 'yyyymmdd'), datestr( end_date, 'yyyymmdd') ), @isstr );
p.addParameter( 'rtc_modes', 0:2, @isnumeric );
p.addParameter( 'rtc_param', 0:20, @isnumeric );
p.parse( varargin{:} );

% -- get threshold from the config structure
try
    threshold = cnf.optim.exceedance_thresholds.( pol_name ).( agg_str );
catch ME
    threshold = -1;
end

if threshold < 0
    threshold = NaN;
    warning( 'OPAQ:OptimThresholdWarning', 'No threshold given for pollutant/aggregation time' );
end
          
% -- open XML file to store the tune results
docNode = com.mathworks.xml.XMLUtils.createDocument('tune');
tuneNode = docNode.getDocumentElement;
tuneNode.setAttribute( 'pollutant', pol_name );
tuneNode.setAttribute( 'aggr', agg_str );
tuneNode.setAttribute( 'mode', cnf.optim.mode );

% -- looping over the stations 
for i=1:length(cnf.network)
    s = cnf.network(i);

    % -- we have requested a particular station list, but this is not one of them...
    if ~isempty( p.Results.stations ) && ~ismember( s.name, p.Results.stations ), continue; end;
    
    % -- does the station measure the requested pollutant ?
    if ~any( strcmp( s.pollutants, pol_name ) ), continue; end;

    fprintf( 'Optimizing station %s....\n', s.name );
    
    
    % -- now write out the infor for the models
    stNode = docNode.createElement( 'station' );
    stNode.setAttribute( 'name', s.name );
    tuneNode.appendChild( stNode );
            
    
    % -- loop over the forecast horizons
    for k=1:length( p.Results.fc_hor )
        fc_hor = p.Results.fc_hor(k);                       
        
        % -- reset the statistics criterium
        best_stat = struct();
        have_data = false;
        
        % -- create the samples for every model
        for j=1:length( p.Results.models )
            model_name = p.Results.models{j};
   
            try
                [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, s.name, pol_name, ...
                    agg_str, p.Results.mor_agg, fc_hor, model_name, start_date, end_date );
            catch ME
                if strcmp( ME.identifier, 'OPAQ:ArchitectureNotFound' )
                    fprintf( 'networks architecture not found, training...\n' );
                    % we have no architecture yet, train the model first !
                    opaq_trainnn( cnf, s.name, pol_name, agg_str, p.Results.mor_agg, fc_hor, ...
                        model_name, start_date, end_date );
                    
                    % and retry the simm !
                    [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, s.name, pol_name, ...
                        agg_str, p.Results.mor_agg, fc_hor, model_name, start_date, end_date );    
                                        
                elseif strcmp( ME.identifier, 'OPAQ:DataNotFound' )
                    fprintf( '+++ no forecasted data available, skipping...\n' );                    
                    continue;
                else
                    rethrow(ME);
                end                
            end
            
            % -- here we have some values, now scan the rtc coefficients...
            if isempty( xx_date )
                fprintf( '+++ no forecasted data available, skipping...\n' );
                continue;
            end
            
            
            % -- scanning the RTC modes/params...
            for ii=1:length(p.Results.rtc_modes)
                mode = p.Results.rtc_modes(ii);
                
                for jj=1:length(p.Results.rtc_param)                    
                    param = p.Results.rtc_param(jj);                                   
                    
                    % Now run over RTC parameters                    
                    rtc_value = opaq_rtc( xx_date, xx_value, fc_value, fc_hor, 'rtc_mode', mode, 'rtc_param', param );
                    
                    % get the validation statistics
                    stat = validstats( xx_value, rtc_value, 'exceedThresh', threshold );                    
                    if ~isfield( stat, cnf.optim.mode )
                        error( 'OPAQ:OptimError', 'statistics do not contain the optimizer mode' );
                    end
                    
                    fprintf( 'scanning mode %d, param %d, %s = %f\n', mode, param, cnf.optim.mode, stat.( cnf.optim.mode ) );
                    
                    if isempty( fieldnames(best_stat) )
                        % store the first structure
                        best_stat  = stat;
                        best_param = struct( 'model', model_name, 'mode',  mode, 'param', param );
                        have_data = true;
                    else
                        try
                            if opaq_isbetter( best_stat, stat, cnf.optim.mode )
                                best_stat = stat;
                                best_param = struct( 'model', model_name, 'mode',  mode, 'param', param );
                            end
                        catch ME
                            rethrow(ME);
                        end
                    end
                    
                    % TODO : clean this up
                    % if mode is not 2, scanning the parameters doesn't
                    % make much sense...
                    if mode ~= 2, break; end
                end
            end
            
        end
        
        % -- some feedback
        if have_data
            fprintf( 'optim %s, day%d : model=%s, rtc=%d, param=%d\n', s.name, fc_hor, ...
                best_param.model, best_param.mode, best_param.param );
            
            modNode = docNode.createElement( 'model' );
            modNode.setAttribute( 'fc_hor', num2str(fc_hor) );
            modNode.setAttribute( 'rtc_mode',  num2str(best_param.mode) );
            modNode.setAttribute( 'rtc_param', num2str(best_param.param) );
            modNode.appendChild( docNode.createTextNode( best_param.model ) );
            stNode.appendChild( modNode );
        end
    end
    
end

% now write the XML file
xmlwrite( p.Results.tune_file, docNode );
