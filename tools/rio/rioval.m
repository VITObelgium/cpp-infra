%rioval RIO Validation tool
%
% Usage : see rioval --help
%
% Developed by Bino Maiheu, (c) VITO 2010-2013
%
%
% RIO (c) VITO/IRCEL 2004-2013
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rioval( varargin )
tic;
argopts = [ struct( 'name', 'start', 'default', -1, 'cast', @(x)datenum(x,31)), ...
    struct( 'name', 'stop', 'default', -1, 'cast', @(x)datenum(x,31)), ...
    struct( 'name', 'conf', 'default', 'base', 'cast', @(x)(x)), ...
    struct( 'name', 'setup_file', 'default', 'rio_setup.xml', 'cast', @(x)(x)), ...
    struct( 'name', 'mode', 'default', 'RIO', 'cast', @(x)(x)), ...
    struct( 'name', 'output', 'default', 'hdf5', 'cast', @(x)(x) ), ...
    struct( 'name', 'help', 'default', false, 'cast', NaN ) ]; % just a switch

% Parse and some initial checks!
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;
if length(args) < 3, error( 'Error in arguments, try --help.' ); end;
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
  
fprintf( 'Requested pollutant          : %s\n', pollutant );
fprintf( 'Requested aggregation time   : %s\n', agg_timestr );
fprintf( 'Requested spatial proxy      : %s\n', proxy );
fprintf( 'Requested interpolation mode : %s\n', opts.mode );
fprintf( 'Requested output mode        : %s\n', opts.output );
fprintf( 'Using configuration          : %s\n', opts.conf );

%% Initialise RIO library...
cnf = rio_setup( opts.setup_file, opts.conf, pollutant, agg_timestr, proxy ); 
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

fprintf( 'Finished initialisation in %f sec.\n', toc );

%% Validation loop
fprintf( '\n' );
fprintf( '                                                           (((\n');
fprintf( '                                                        \\ (. .) /\n');
fprintf( '                                                         (( v ))\n');
fprintf( '-----------------------------------------------------------m-m--------\n' );
fprintf( ' Starting cross-validation...\n');
fprintf( '----------------------------------------------------------------------\n' );
tic;
xx_val = struct();
for st_id=1:cnf.nr_st

  have_station = true;
  
  % first check whether our station is not flagged in the xml file
  % normally data is get rid of in rio_dblookup,
  % need to change this at some point to make things a bit more
  % efficient...
  if isfield( cnf, 'stationList' ) && ~isempty( cnf.stationList )
    if ~any( find( strcmp( cnf.stationList, cnf.st_id{ st_id } ) ) )
      fprintf( '+++ EXPERIMENTAL: skipping %s based upon stationList in XML\n', cnf.st_id{ st_id } );
      have_station = false;
    else
      fprintf( '+++ EXPERIMENTAL: validating %s...\n', cnf.st_id{ st_id } );
    end
  end
  
  % do the validation...
  if have_station
    [ s, mod, obs ] = rio_validate( cnf, st_id, xx_dates );  
  else
    mod = [];
    obs = [];
  end
  
  % set missing values...
  mod( isnan( mod ) ) = cnf.missing_value;
  obs( isnan( obs ) ) = cnf.missing_value;
  
  % output....  
  xx_val(st_id).name         = cnf.st_id{ st_id };
  xx_val(st_id).type         = cnf.st_info( st_id, 5 );
  xx_val(st_id).indic        = cnf.st_indic( st_id );
  xx_val(st_id).x            = cnf.st_info( st_id, 2 );
  xx_val(st_id).y            = cnf.st_info( st_id, 3 );
  xx_val(st_id).have_station = have_station;
  xx_val(st_id).xx_mod = mod;
  xx_val(st_id).xx_obs = obs;
end
% transpose data array
xx_dates = xx_dates';

% creating output file --> txt or h5...
if strcmpi( opts.output, 'hdf5' ) || strcmpi( opts.output, 'both' )
  h5name = fullfile( cnf.outputPath, sprintf( 'rioval_%s_%s_%s_%s-%s.h5', ...
    cnf.pol_xx, cnf.at_lb, cnf.gis_type, ...
    datestr( xx_dates(1), 'yyyymmdd' ), datestr( xx_dates(end), 'yyyymmdd') ) );
  
  fprintf( 'Writing output to %s...\n', h5name );
  if exist( h5name, 'file' )
    fprintf( 'ERROR: %s already exists, dumping matlab file...\n', h5name );
    fprintf( 'ERROR: pleas use rioval_export to convert to hdf5....\n' );
    save( 'rioval_export.mat', 'cnf', 'opts', 'xx_dates', 'xx_val' );
    error( 'Stopping here !' );
  else
    rioval_export( h5name, cnf, opts, xx_dates, xx_val );
  end
else
  warning( 'output mode not supported yet, dumping matlab file...' );
  save( 'rioval_export.mat', 'cnf', 'opts', 'xx_dates', 'xx_val' );
end





fprintf( 'Finished validating in %f sec.\n', toc );
fprintf( '\n' );
fprintf( '**********************************************************************\n' );
fprintf( '*    (((                                                             *\n');
fprintf( '*   (. .)          All done.                                         *\n');
fprintf( '*  (( v ))                                                           *\n');
fprintf( '* ---m-m---        H A V E   A   N I C E   D A Y  ! ! !              *\n' );
fprintf( '**********************************************************************\n' );


function print_header
v = rio_version;
fprintf( '                                                    (((\n');
fprintf( '                                                   (. .)\n');
fprintf( '                                                  (( v ))\n');
fprintf( '************************************************ ---m-m--- ***********\n' );
fprintf( '*                         Welcome to the RIO                         *\n' );
fprintf( '*                         Validation Toolkit                         *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*         RIO (c) VITO/IRCEL 2004-2013                               *\n' );
fprintf( '*         Jef Hooybergs, Stijn Janssen, Frans Fierens                *\n' );
fprintf( '*         Nele Veldeman, Bino Maiheu                                 *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*   Contact: bino.maiheu@vito.be                                     *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*   Library version : %d.%d                                            *\n', v.major, v.minor );
fprintf( '**********************************************************************\n' );

function print_usage
% Prints a small usage message...
fprintf( 'Usage: \n' );
fprintf( '  rioval [options] <pol> <agg_time> <proxy>\n' );
fprintf(' Where : \n' );
fprintf('    pol ......... : pm10, pm25, no2, o3, so2, nh3 \n' );
fprintf('    agg_time .... : m1, m8, da or 1h  \n' );
fprintf('    proxy ....... : CorineID, CorineID_double_beta, AOD, ... \n' );
fprintf('\n' );
fprintf(' Optional arguments, prefixed by "--" :\n' );
fprintf('    --help                  : this message\n' );
fprintf('    --output <mode>         : hdf5/txt/both/nc (def: hdf5)\n' );
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
fprintf('RIO (c) VITO/IRCEL 2004-2013\n');
fprintf('Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu\n');
