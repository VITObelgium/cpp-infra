%RIO Main program for rio interpolation model.
%
% Usage : please see "rio --help"
%
% RIO (c) VITO/IRCEL 2004-2012 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function  rio( varargin )
%% Configuration
tic;
argopts = [ struct( 'name', 'start', 'default', -1, 'cast', @(x)datenum(x,31)), ...
    struct( 'name', 'stop', 'default', -1, 'cast', @(x)datenum(x,31)), ...
    struct( 'name', 'conf', 'default', 'base', 'cast', @(x)(x)), ...
    struct( 'name', 'setup_file', 'default', 'rio_setup.xml', 'cast', @(x)(x)), ...
    struct( 'name', 'mode', 'default', 'RIO', 'cast', @(x)(x)), ...
    struct( 'name', 'output', 'default', 'txt', 'cast', @(x)(x) ), ...
    struct( 'name', 'help', 'default', false, 'cast', NaN ) ]; % just a switch

% Parse and some initial checks!
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;
if length(args) < 4, error( 'Error in arguments, try --help.' ); end;
if ~( strcmpi( opts.output, 'hdf5' ) || strcmpi( opts.output, 'txt' ) ||...
    strcmpi( opts.output, 'both' ) )
error( 'Unknown output mode : %s, please select [hdf5/txt/both]', opts.output );
end

%-- Welcome !
print_header;
fprintf( '\n' );
fprintf( '                                               (((\n');
fprintf( '                                              (. .)\n');
fprintf( '                                            <(( v ))>\n');
fprintf( '-----------------------------------------------m-m--------------------\n' );
fprintf( ' Initialisation...\n' );
fprintf( '----------------------------------------------------------------------\n' );

%-- Checking arguments
pollutant   = args{1};
agg_timestr = args{2};
proxy       = args{3};
grid_str    = args{4};
  
fprintf( 'Requested pollutant          : %s\n', pollutant );
fprintf( 'Requested aggregation time   : %s\n', agg_timestr );
fprintf( 'Requested spatial proxy      : %s\n', proxy );
fprintf( 'Requested interpolation grid : %s\n', grid_str );
fprintf( 'Requested interpolation mode : %s\n', opts.mode );
fprintf( 'Requested output mode        : %s\n', opts.output );
fprintf( 'Using configuration          : %s\n', opts.conf );

%% Initialise the RIO library...
%cnf = rio_init( path, pollutant, agg_timestr, proxy, grid_str, opts.mode );
cnf = rio_setup( opts.setup_file, opts.conf, pollutant, agg_timestr, proxy, grid_str );
if cnf.errcode
   error( 'rio:: %s', cnf.errmsg );
else
    fprintf( 'Library v%d.%d initialisation OK\n', cnf.version.major, cnf.version.minor );    
    % Set the deployment for this executable, this will influence the checks
    % performed in the rio_checkdeployment routine
    cnf.deployment = ''; % add later : IRCEL, VMM-NH3, RIVM, CHINA etc...
    cnf.ipol_mode  = opts.mode;
end

% Check some deployment specific parameters etc...
cnf = rio_checkdeployment( cnf );
if cnf.errcode, 
    error( 'rio::deployment check failed...' ); 
end;

% Load the station info
cnf = rio_loadstationinfo( cnf );
if cnf.errcode,  error( 'rio:: %s', cnf.errmsg ); end;

% Load the gridinfo
cnf = rio_loadgrid( cnf );
if cnf.errcode, error( 'rio:: %s', cnf.errmsg ); end;


% TODO read from XML
if strcmp( cnf.pol, 'bc' )
    cnf.scale_factor    = 0.01;
    cnf.detection_limit = 0.01;
else
    cnf.scale_factor    = 1.0;
    cnf.detection_limit = 1.0;
end


%% Load some data, or historic database...
fprintf( 'Scanning for concentration data...\n' );
% if a <pol>_data_rio.txt file is available, rio will read it's input from
% there, to mimic the fortran version, if not, we will try to load the
% matlab database
ascii_input = fullfile( cnf.dbasePath, sprintf( '%s_data_rio.txt', cnf.pol_xx ) );
if exist( ascii_input, 'file' )
    fprintf( 'Found %s, importing measurments from ASCII file...\n', ascii_input );
    cnf = rio_createdb( cnf, ascii_input, false );
else
    fprintf( 'Importing measurements from historic matlab database...\n' );
    cnf = rio_loaddb( cnf );
    if cnf.errcode, error( 'rio:: %s', cnf.errmsg ); end;
end

% Set start/stop time for interpolation based on the loaded archive
% if we have start/stop equal to -1 : then we use the full range
% of the archive to interpolate...
% if the user give a 5th fixed argument -> interpret as time for a single
% interpolation
xx_dates = [];
if numel(args) == 5    
    fprintf( 'Requested interpolation %s\n', datestr(xx_dates) );
    if cnf.agg_time <= 3
        xx_dates = datenum(args{5},31);
    else 
        % make sure we have the 24 hours in 1h mode for the requested day...
        time_step  = datenum(0,0,0,1,0,0);
        xx_dates = datenum(args{5},31):time_step:datenum(args{5},31)+datenum(0,0,0,23,59,59);
    end
else
    fprintf( 'Requested interpolation range :\n' );
    if opts.start <= 0
        start_time   = min(cnf.xx_date);
    else
        start_time  = opts.start;
    end
    if opts.stop <= 0
        end_time   = max(cnf.xx_date)+datenum(0,0,0,23,59,59);
    else
        end_time  = opts.stop;
    end
    
    fprintf( ' from : %s\n', datestr( start_time ) );
    fprintf( ' to   : %s\n', datestr( end_time ) );
    if cnf.agg_time <= 3
        time_step  = datenum(0,0,1,0,0,0);
    else
        time_step  = datenum(0,0,0,1,0,0);
    end
    fprintf( ' step : %f day\n', time_step );
    xx_dates = start_time:time_step:end_time;
end

% Apply data quality checks, e.g. drop periods with high PM10 for IRCEL or
% drop winter data for o3s, this routine internally checks what deployment
% you're working in, as data quality checks can differ for different 
% implementations
cnf = rio_dbdaql( cnf );


% apply the log transformation outside of the load/create database routines
% also best do this after the data quality checks...
if cnf.Option.logtrans
  fprintf( 'Applying log transformation to measurement database...\n' );
  cnf.xx_val(:,2:end) = log( 1. + cnf.xx_val(:,2:end) );
end


% Opening hdf5 file
if strcmpi( opts.output, 'hdf5' ) || strcmpi( opts.output, 'both' )
    if numel(xx_dates) == 1
        h5name = fullfile( cnf.outputPath, sprintf( 'rio_%s_%s_%s_%s_%s.h5', ...
            cnf.pol_xx, cnf.at_lb, cnf.gis_type, cnf.grid_type, ...
            datestr( xx_dates, 'yyyymmdd' ) ) );
    else
        h5name = fullfile( cnf.outputPath, sprintf( 'rio_%s_%s_%s_%s_%s-%s.h5', ...
            cnf.pol_xx, cnf.at_lb, cnf.gis_type, cnf.grid_type, ...
            datestr( xx_dates(1), 'yyyymmdd' ), datestr( xx_dates(end), 'yyyymmdd') ) );
    end
    fprintf( 'Creating hdf5 output in : %s\n', h5name );
    fh = rio_h5create( cnf,  h5name );
end
fprintf( 'Finished initialisation in %f sec.\n', toc );
		
%% Interpolation loop
fprintf( '\n' );
fprintf( '                                                           (((\n');
fprintf( '                                                        \\ (. .) /\n');
fprintf( '                                                         (( v ))\n');
fprintf( '-----------------------------------------------------------m-m--------\n' );
fprintf( ' Interpolating...\n');
fprintf( '----------------------------------------------------------------------\n' );
tic;
%-- Loop over all 'time step' values between start and stop date...
for date = xx_dates
    
    fprintf( 'Interpolating %s\n', datestr( date ) );

    %-- Update the interpolation parameters : week/weekend, time of the day
    %-- So re-load the spatial correlations, trend parameters etc...
    cnf = rio_updatepars( cnf, date );
    
    %-- Interpolate for the requested date, trimming of the data is handle internally...    
    [ vals, grid ] = rio_interpolate( cnf, date );  
    
    %-- Now transform the concentrations back to normal ones if we have
    %   applied the log transform...
    if cnf.Option.logtrans      
      %  transform measurements back to normal values before writing to
      %  file
      vals( vals(:,2) ~= cnf.missing_value, 2 ) = exp( vals( vals(:,2) ~= cnf.missing_value, 2 ) ) - 1;
      
      % idem for gridded concentrations, do the error first otherwise grid
      % is overwritten...
      grid( grid(:,2) ~= cnf.missing_value, 3 ) = exp( grid( grid(:,2) ~= cnf.missing_value, 2 ) ) .* ...
        grid( grid(:,2) ~= cnf.missing_value, 3 );
      
      grid( grid(:,2) ~= cnf.missing_value, 2 ) = exp( grid( grid(:,2) ~= cnf.missing_value, 2 ) ) - 1;      
    end
                
    %-- set concentrations < 1 to 1
    grid( grid(:,2)<cnf.detection_limit, 2 ) = cnf.detection_limit;
    
    %-- Now store the values and the grid somewhere...    
    if strcmpi( opts.output, 'txt' ) || strcmpi( opts.output, 'both' )
        cnf = rio_output( cnf, vals, grid, 'RIO', date );
    end
		        
    
	%-- Append to the HDF5 output file
    if strcmpi( opts.output, 'hdf5' ) || strcmpi( opts.output, 'both' )
        fh = rio_h5append( cnf, fh, date, vals, grid );
    end
end

if strcmpi( opts.output, 'hdf5' ) || strcmpi( opts.output, 'both' )
    fprintf( 'Closing hdf5 output %s\n', h5name );
    rio_h5close( fh );
end
fprintf( 'Finished interpolating in %f sec.\n', toc );
fprintf( '\n' );
fprintf( '**********************************************************************\n' );
fprintf( '*    (((                                                             *\n');
fprintf( '*   (. .)          All done.                                         *\n');
fprintf( '*  (( v ))                                                           *\n');
fprintf( '* ---m-m---        H A V E   A   N I C E   D A Y  ! ! !              *\n' );
fprintf( '**********************************************************************\n' );


function print_usage
% Prints a small usage message...
fprintf( 'Usage: \n' );
fprintf( '  rio [options] <pol> <agg_time> <proxy> <grid> [date]\n' );
fprintf(' Where : \n' );
fprintf('    pol ......... : pm10, pm25, no2, o3, so2, nh3 \n' );
fprintf('    agg_time .... : m1, m8, da or 1h  \n' );
fprintf('    proxy ....... : CorineID, CorineID_double_beta, AOD, ... \n' );
fprintf('    grid ........ : 4x4, 1x1, belEUROS, ... \n' );
fprintf('\n' );
fprintf(' Optional arguments, prefixed by "--" :\n' );
fprintf('    --help                  : this message\n' );
fprintf('    --output <mode>         : hdf5/txt/both (def: txt)\n' );
fprintf('    --start <time> ........ : start date/time for interpolation of a series.\n' );
fprintf('    --stop <time> ......... : end date/time for interpolation of a series.\n' );
fprintf('    --conf <name> ......... : configuration name, selects XML config section, def: base\n');
fprintf('    --mode <ipol_mode> .... : interpolation mode : RIO (def.), OrdKrig, IDW, IDWi, ... \n' );
fprintf('    --setup_file <fname> .. : alternative setup file, default: rio_setup.xml\n');
fprintf('\n' );
fprintf(' Additional comments:\n');
fprintf('   - In order to specify a date, the format has to be readable by the matlab\n');
fprintf('     datenum(x,31) function, this means : "yyyy-mm-dd HH:MM:SS"\n');
fprintf('   - If a date is specified by the optional final fixed argument, this will take\n');
fprintf('     precedence over a range specified by --start and --stop arguments.\n');
fprintf('\n' );
fprintf('RIO (c) VITO/IRCEL 2004-2015\n');
fprintf('Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu\n');

function print_header
v = rio_version;
fprintf( '                                                    (((\n');
fprintf( '                                                   (. .)\n');
fprintf( '                                                  (( v ))\n');
fprintf( '************************************************ ---m-m--- ***********\n' );
fprintf( '*                           Welcome to RIO                           *\n' );
fprintf( '*                An Air Quality Interpolation Model                  *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*         RIO (c) VITO/IRCEL 2004-2012                               *\n' );
fprintf( '*         Jef Hooybergs, Stijn Janssen, Frans Fierens                *\n' );
fprintf( '*         Nele Veldeman, Bino Maiheu                                 *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*   Contact: bino.maiheu@vito.be                                     *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*   Library version : %d.%d                                            *\n', v.major, v.minor );
fprintf( '**********************************************************************\n' );


