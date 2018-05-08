% OPAQ_FORECAST General purpose forcast/validation tool
%   
%   opaq_forecast( cnf, pol_name, agg_str, start_date, end_date )
%   opaq_forecast( cnf, pol_name, agg_str, start_date, end_date, varargin )
%
% General purpose forecast routine. Includes functionality to compute
% validation for the forecast as well. This forecast routine produces
% the full timeseries retrospectively. For real-time forecasting use opaqfc. 
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
%  'mode', <mode>  : 'ann'   : perform purely a ANN forecast for the given
%                              models (default !!)
%                    'rtc'   : perform a real time corrected forecast using
%                              the given parameters
%                    'batch' : perform a batch forecast using the tune
%                              file
%
%  'rtc_mode', <mode>        : correct each ann forecast model using this rtc_mode
%  'rtc_param', <value>      : using this parameter
%
%  'tune_file', <name>       : use this tunefile
%
%  'optim_mode', <name>      : use this optimization mode, default 'rmse'
%
%  'valid', true/false       : perform the validation as well
%
%  'valid_file', <name>      : write the validation results to this file
%
%  'valid_indic', { 'rmse', 'bias', ... } : list of indicators to write in the 
%                              validation. 
%                              Default are : rmse, bias, r2, fcf, ffa
%
%  'valid_plot', true/false  : make & store a validation plot
%  'valid_plotdir', <name>   : output location for plots, default .
%
%  'output', <mode>          : dump the output into this folder, default
%                             'none', supported are : 
%                               - 'none'   : no output (default)
%                               - 'text'   : produce text files for xl
%                               - 'matlab' : write matlab files
%                               - 'hdf5'   : write a big HDF5 file
%                               - 'delta'  : write delta tool input format
%
% See also opaqfc
%
% Author: Bino Maiheu
% Contact : bino.maiheu@vito.be

function opaq_forecast( cnf, pol_name, agg_str, start_date, end_date, varargin )

% -- parse command line arguments
p = inputParser;
p.CaseSensitive = true;
p.addParameter( 'fc_hor', 0:cnf.fc_hor_max, @isnumeric );
p.addParameter( 'stations', {}, @iscell );
p.addParameter( 'mor_agg', 9, @isnumeric );
p.addParameter( 'mode', 'ann', @isstr );
p.addParameter( 'models', opaq_getmodels, @(x) assert( iscell(x) && ~isempty(x) ) );
p.addParameter( 'rtc_mode', 0, @(x)assert( isnumeric(x) && isscalar(x) ) );
p.addParameter( 'rtc_param', 0, @(x) assert( isnumeric(x) && isscalar(x) ) );
p.addParameter( 'tune_file', '', @(x) assert( ischar(x) && ~isempty(x) ) );
p.addParameter( 'optim_mode', 'rmse', @ischar );
p.addParameter( 'valid', false, @islogical );
p.addParameter( 'valid_file', sprintf( 'opaq_valid_%s_%s-%s-%s.txt', ...
    pol_name, agg_str, datestr( start_date, 'yyyymmdd' ), datestr( end_date, 'yyyymmdd' )  ) , @ischar );
p.addParameter( 'valid_indic', { 'rmse', 'bias', 'r2', 'fcf', 'ffa' }, @iscell );
p.addParameter( 'valid_plot', false, @islogical );
p.addParameter( 'valid_plotdir', '.', @ischar );
p.addParameter( 'output', 'none', @ischar );

% -- parse the command line arguments
p.parse( varargin{:} );

% -- opening the tune
try
    tuneConfig = parseChildNodes( xmlread( p.Results.tune_file ) );
catch ME
    if strcmp( p.Results.mode, 'batch' )
        error( 'OPAQ:TuneImportError', 'Cannot import the tune file : %s (%s)', ...
            p.Results.tune_file, ME.message );
    end
end

if p.Results.valid
    % -- open validation file
    if exist( p.Results.valid_file, 'file' )
        error( 'OPAQ:FileExists', 'Validation file already present, clean up first' );
    else
        fid = fopen( p.Results.valid_file, 'wt' );
        if fid < 0,
            error( 'OPAQ:FileOpenError', 'Cannot open validation file...' );
        end
    end
    
    % header
    fprintf( fid, 'POL_NAME\tAGGR_STR\tMORG_AGG\tSTATION\tFC_HOR\tMODEL\tRTC_MODE\tRTC_PARAM' );
    for k=1:length( p.Results.valid_indic )
        fprintf( fid, '\t%s', upper( p.Results.valid_indic{k} ) );
    end
    fprintf( fid, '\n' );
    
    % -- get threshold from the config structure
    try
        threshold = cnf.optim.exceedance_thresholds.( pol_name ).( agg_str );
    catch ME
        threshold = -1;
    end
end

% -- looping over the stations 
for i=1:length(cnf.network)
    s = cnf.network(i);

    % -- we have requested a particular station list, but this is not one of them...
    if ~isempty( p.Results.stations ) && ~ismember( s.name, p.Results.stations ), continue; end;
    
    % -- does the station measure the requested pollutant ?
    if ~any( strcmp( s.pollutants, pol_name ) ), continue; end;
            
    
    fprintf( 'Forecasting %s...\n', s.name );
    
    % -- loop over the forecast horizons
    for k=1:length( p.Results.fc_hor )
        fc_hor = p.Results.fc_hor(k);
            

        % -- prepare output
        if ~strcmp( p.Results.output, 'none' )
            output_path = opaq_strrep( cnf.io.outdir_pattern, 'pol', pol_name, 'agg_str', agg_str, 'fc_hor', fc_hor );
            output_base = opaq_strrep( cnf.io.outfile_pattern, 'pol', pol_name, 'agg_str', agg_str, 'fc_hor', fc_hor, 'station', s.name, 'mor_agg', p.Results.mor_agg );
            if ~exist( output_path, 'dir' ), mkdir(  output_path ); end;
        end
                
        
        
        if strcmp( p.Results.mode, 'batch' )
            % ------------------------------
            % do a batch forecasts
            % ------------------------------            
            
            % get the tune from the file
            tune = opaq_readtune( tuneConfig, p.Results.optim_mode, pol_name, agg_str, s.name, fc_hor );
        
            try
                % produce the ann forecast
                [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, s.name, pol_name, ...
                    agg_str, p.Results.mor_agg, fc_hor, tune.model_name, start_date, end_date );
                            
                % run the rtc with the tuned mode & parameter
                rtc_value = opaq_rtc( xx_date, xx_value, fc_value, fc_hor, ...
                    'rtc_mode', tune.rtc_mode, 'rtc_param', tune.rtc_param );
                
            catch ME
                switch ME.identifier
                    case 'OPAQ:DataNotFound'
                        fprintf( 'day%d... no data found\n', fc_hor );
                        continue; % skipping                        
                    otherwise
                        rethrow(ME);
                end
            end
                        
            % -- do the validation
            if p.Results.valid
                if p.Results.valid_plot
                    label = sprintf( '%s_%s_mor%d_%s_day%d_%s_rtc%d', pol_name, agg_str, ...
                        p.Results.mor_agg, s.name, fc_hor, tune.model_name, tune.rtc_mode );
                    [ stats, hFig ] = validstats( xx_value, rtc_value, 'exceedThresh', threshold, 'makePlot', true, ...
                        'obsLabel', pol_name, ...
                        'modLabel', sprintf( '%s - RTC%d (%d)', tune.model_name, tune.rtc_mode, tune.rtc_param ), ...
                        'plotTitle', strrep( label, '_', '\_' ) );
                    saveas( hFig, fullfile( p.Results.valid_plotdir, sprintf( '%s.png', label ) ) );
                    close(hFig);
                else
                    stats = validstats( xx_value, rtc_value, 'exceedThresh', threshold );
                end
                 
                fprintf( fid, '%s\t%s\t%d\t%s\t%d\t%s\t%d\t%d', pol_name, agg_str, ...
                    p.Results.mor_agg, s.name, fc_hor, tune.model_name, tune.rtc_mode, tune.rtc_param  );
                for ii=1:length( p.Results.valid_indic )
                    fprintf( fid, '\t%f', stats.( p.Results.valid_indic{ii} ) );
                end
                fprintf( fid, '\n' );
            end
            
            
            % -- write the forecast timeseries results                            
            switch( p.Results.output )
                case 'none' % do nothing
                    
                case 'text'
                    dv = datevec( xx_date );
                    ofd = fopen( fullfile( output_path, sprintf( '%s.txt', output_base ) ), 'wt' );
                    fprintf( ofd, 'FC_DATE\tFC_HOR\tOBS\tFC\tRTC\n' );                    
                    fprintf( ofd, '%04d%02d%02d\t%d\t%.2f\t%.2f\t%.2f\n', [ dv(:,1) dv(:,2) dv(:,3) repmat(fc_hor,length(xx_date),1) xx_value fc_value rtc_value ]' );
                    fclose(ofd);
                    
                case 'matlab'
                    save( fullfile( output_path, sprintf( '%s.mat', output_base ) ), 'xx_date', 'xx_value', 'fc_value', 'rtc_value' );
                
                case 'hdf5'
                    fname = fullfile( output_path, sprintf( '%s.h5', output_base ) );
                    
                    h5create( fname, '/xx_date', size( xx_date ) );
                    h5create( fname, '/xx_value', size( xx_date ) );
                    h5create( fname, '/fc_value', size( xx_date ) );
                    h5create( fname, '/rtc_value', size( xx_date ) );
                    
                    h5write( fname, '/xx_date', xx_date );
                    h5write( fname, '/xx_value', xx_value );
                    h5write( fname, '/fc_value', fc_value );
                    h5write( fname, '/rtc_value', rtc_value );
                    
                    % write some attributes                    
                    h5writeatt( fname, '/', 'pollutant', pol_name );
                    h5writeatt( fname, '/', 'mode', p.Results.mode );
                    h5writeatt( fname, '/', 'station', s.name );
                    h5writeatt( fname, '/', 'agg_str', agg_str );
                    h5writeatt( fname, '/', 'mor_agg', p.Results.mor_agg );
                    h5writeatt( fname, '/', 'fc_hor', sprintf( 'day%d', fc_hor ) );
                    h5writeatt( fname, '/', 'tune_file', tune_file );
                    h5writeatt( fname, '/', 'model', tune.model_name );
                    h5writeatt( fname, '/', 'rtc_mode', tune.rtc_mode );
                    h5writeatt( fname, '/', 'rtc_param', tune.rtc_param );
                                        
                otherwise
                    error( 'OPAQ:ConfigError', 'Output mode %s not implemented yet', p.Results.output );
            end                                                            
            
        else
            % ------------------------------
            % loop over all the given models
            % ------------------------------
            for j=1:length( p.Results.models )
                model_name = p.Results.models{j};
        
                try
                    % produce the ann forecast
                    [ xx_date, xx_value, fc_value ] = opaq_simnn( cnf, s.name, pol_name, ...
                        agg_str, p.Results.mor_agg, fc_hor, model_name, start_date, end_date );
                
                    if strcmp( p.Results.mode, 'rtc' )
                        % correct using the given rtc mode & parameter
                        rtc_mode  = p.Results.rtc_mode;
                        rtc_param = p.Results.rtc_param;
                        rtc_value = opaq_rtc( xx_date, xx_value, fc_value, fc_hor, ...
                            'rtc_mode', rtc_mode, 'rtc_param', rtc_param );
                    else
                        rtc_value = fc_value;
                        rtc_mode  = 0;
                        rtc_param = 0;
                    end
                    
                catch ME
                    switch ME.identifier
                        case 'OPAQ:DataNotFound'
                            continue; % skipping
                    otherwise
                        rethrow(ME);
                    end
                end

            
                % do validation ?
                if p.Results.valid
                    
                    if p.Results.valid_plot
                        label = sprintf( '%s_%s_mor%d_%s_day%d_%s_rtc%d', pol_name, agg_str, ...
                            p.Results.mor_agg, s.name, fc_hor, model_name, rtc_mode );
                        [ stats, hFig ] = validstats( xx_value, rtc_value, 'exceedThresh', threshold, 'makePlot', true, ...
                            'obsLabel', pol_name, ...
                            'modLabel', sprintf( '%s - RTC%d (%d)', model_name, rtc_mode, rtc_param ), ...
                            'plotTitle', strrep( label, '_', '\_' ) );
                        saveas( hFig, fullfile( p.Results.valid_plotdir, sprintf( '%s.png', label ) ) );
                        close(hFig);
                    else
                        stats = validstats( xx_value, rtc_value, 'exceedThresh', threshold );
                    end
                    
                     fprintf( fid, '%s\t%s\t%d\t%s\t%d\t%s\t%d\t%d', pol_name, agg_str, ...
                         p.Results.mor_agg, s.name, fc_hor, model_name, rtc_mode, rtc_param  );
                     for ii=1:length( p.Results.valid_indic )
                         fprintf( fid, '\t%f', stats.( p.Results.valid_indic{ii} ) );
                     end
                     fprintf( fid, '\n' );                    
                end
                
                % -- append the results, for matlab we make separaet files
                switch( p.Results.output )
                    case 'none' % do nothing
                        
                    case 'text'
                        dv = datevec( xx_date );                        
                        ofd = fopen( fullfile( output_path, sprintf( '%s-%s.txt', output_base, model_name ) ), 'wt' );
                        fprintf( ofd, 'FC_DATE\tFC_HOR\tOBS\tFC\tRTC\n' );
                        fprintf( ofd, '%04d%02d%02d\t%d\t%.2f\t%.2f\t%.2f\n', [ dv(:,1) dv(:,2) dv(:,3) repmat(fc_hor,length(xx_date),1) xx_value fc_value rtc_value ]' );
                        fclose(ofd);
                        
                    case 'matlab'                        
                        save( fullfile( output_path, sprintf( '%s-%s.mat', output_base, model_name ) ), ...
                            'xx_date', 'xx_value', 'fc_value', 'rtc_value' );
                        
                    case 'hdf5'
                        % use same output file for all models, store models
                        % in different groups
                        fname = fullfile( output_path, sprintf( '%s.h5', output_base ) );
                    
                        h5create( fname, sprintf( '/%s/xx_date', model_name ), size( xx_date ) );
                        h5create( fname, sprintf( '/%s/xx_value', model_name ), size( xx_date ) );
                        h5create( fname, sprintf( '/%s/fc_value', model_name ), size( xx_date ) );
                        h5create( fname, sprintf( '/%s/rtc_value', model_name ), size( xx_date ) );
                    
                        h5write( fname, sprintf( '/%s/xx_date', model_name ), xx_date );
                        h5write( fname, sprintf( '/%s/xx_value', model_name ), xx_value );
                        h5write( fname, sprintf( '/%s/fc_value', model_name ), fc_value );
                        h5write( fname, sprintf( '/%s/rtc_value', model_name ), rtc_value );
                    
                        % write some attributes
                        h5writeatt( fname, '/', 'pollutant', pol_name );
                        h5writeatt( fname, '/', 'mode', p.Results.mode );
                        h5writeatt( fname, '/', 'station', s.name );
                        h5writeatt( fname, '/', 'agg_str', agg_str );
                        h5writeatt( fname, '/', 'mor_agg', p.Results.mor_agg );
                        h5writeatt( fname, '/', 'fc_hor', sprintf( 'day%d', fc_hor ) );                        
                        h5writeatt( fname, '/', 'rtc_mode', p.Results.rtc_mode );
                        h5writeatt( fname, '/', 'rtc_param', p.Results.rtc_param );
                        
                    otherwise
                        error( 'OPAQ:ConfigError', 'Output mode %s not implemented yet', p.Results.output );
                end
                                    
            end
        
        end
        
    end
end

% -- close validation file
if p.Results.valid
    fclose( fid );
end
