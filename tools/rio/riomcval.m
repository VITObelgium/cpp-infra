%rioval RIO Monte Carlo Validation tool
%
% Usage : see riomcval --help
%
% Developed by Bino Maiheu, (c) VITO 2010-2013
%
%
% RIO (c) VITO/IRCEL 2015
% Bino Maiheu

function riomcval( varargin )
tic;
argopts = [ struct( 'name', 'start', 'default', -1, 'cast', @(x)datenum(x,31)), ...
    struct( 'name', 'stop', 'default', -1, 'cast', @(x)datenum(x,31)), ...
    struct( 'name', 'pct', 'default', 20, 'cast', @(x)str2double(x)), ...
    struct( 'name', 'n_min', 'default', 1, 'cast', @(x)str2double(x)), ...
    struct( 'name', 'conf', 'default', 'base', 'cast', @(x)(x)), ...
    struct( 'name', 'setup_file', 'default', 'rio_setup.xml', 'cast', @(x)(x)), ...
    struct( 'name', 'mode', 'default', 'RIO', 'cast', @(x)(x)), ...
    struct( 'name', 'help', 'default', false, 'cast', NaN ) ]; % just a switch

% Parse and some initial checks!
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;
if length(args) < 3, error( 'Error in arguments, try --help.' ); end;

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
fprintf( 'Using configuration          : %s\n', opts.conf );
fprintf( 'Leaving out pct              : %d %%\n', opts.pct );
fprintf( 'Minimal station selection    : %d\n',    opts.n_min );

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
fprintf( ' Starting Monte Carlo cross-validation...\n');
fprintf( '----------------------------------------------------------------------\n' );
tic;
xx_val = struct();

%% first select the sample of stations we perform the validation on


  % first check whether our station is not flagged in the xml file
  % normally data is get rid of in rio_dblookup,
  % need to change this at some point to make things a bit more
  % efficient...
%   if isfield( cnf, 'stationList' ) && ~isempty( cnf.stationList )
%     if ~any( find( strcmp( cnf.stationList, cnf.st_id{ st_id } ) ) )
%       fprintf( '+++ EXPERIMENTAL: skipping %s based upon stationList in XML\n', cnf.st_id{ st_id } );
%       have_station = false;
%     else
%       fprintf( '+++ EXPERIMENTAL: validating %s...\n', cnf.st_id{ st_id } );
%     end
%   end



for i=1:cnf.nr_st
    xx_val(i).id     = cnf.st_info(i,1);
    xx_val(i).name   = cnf.st_id{i};
    xx_val(i).type   = cnf.st_info(i,5);
    xx_val(i).indic  = cnf.st_indic(i);
    xx_val(i).x      = cnf.st_info(i,2);
    xx_val(i).y      = cnf.st_info(i,3);
    xx_val(i).nval   = 0;
    xx_val(i).xx_mod = [];
    xx_val(i).xx_obs = [];
    
    xx_val(i).rmse = [];
end


%% Now start randomly selectin subsets of these stations
prct  = opts.pct/100;   % we leave out 20% of the stations
n_min = opts.n_min;     %don't stop until each station has been validated at least n_min times

n_sel = min(round(prct*cnf.nr_st),cnf.nr_st );
fprintf( 'Leaving out %d stations of the %d ( %.1f %%)\n', n_sel, cnf.nr_st,  100.*prct );

%% Book some output arrays
n_valids = zeros(cnf.nr_st,1);   % number of validations for each station

validation_iter     = 1;
validation_finished = false;
while ~validation_finished
    
    % perform random permutation of the indices of available stations
    randIdx = randperm(cnf.nr_st);
    st_idx  = randIdx(1:n_sel);
    
    % select the first n_sel statoins for validation (and leaving out)
    st_id = cnf.st_info(st_idx,1);    
    
    % do the validation...    
    [ stat, mod, obs ] = rio_validate( cnf, st_id, xx_dates );  
    
    % set missing values...
    mod( isnan( mod ) ) = cnf.missing_value;
    obs( isnan( obs ) ) = cnf.missing_value;
    
    n_valids( st_idx ) = n_valids( st_idx ) + 1;
    
    % Now we have to select for each station the maximum RMSE    
    
    
    % output....
    for i=1:length(st_idx)
        xx_val(st_idx(i)).xx_mod = [ xx_val(st_idx(i)).xx_mod  mod(:,i) ];
        xx_val(st_idx(i)).xx_obs = [ xx_val(st_idx(i)).xx_obs  obs(:,i) ];
 
        xx_val(st_idx(i)).nval   = xx_val(st_idx(i)).nval + 1;
        xx_val(st_idx(i)).rmse   = [ xx_val(st_idx(i)).rmse stat.rmse(i) ]; 
    end

   
    % validation finished if all stations have been validated at least
    % n_min times    
    if ~any( n_valids < n_min )
        validation_finished = true;
        fprintf( '[riomcval] all stations have been validated %d times, finishing...\n', n_min );
    else
        fprintf( '[riomcval] %d stations have not been validated %d times yet, continuing (iteration %d)... \n', ...
            numel( find( n_valids < n_min ) ), n_min, validation_iter );
    end
    validation_iter = validation_iter + 1;
end

% transpose data array
xx_dates = xx_dates';

matfile = fullfile( cnf.outputPath, sprintf( 'riomcval_%s_%s_%s_%s-%s-n_min%d_%dpct.mat', ...
    cnf.pol_xx, cnf.at_lb, cnf.gis_type, ...
    datestr( xx_dates(1), 'yyyymmdd' ), datestr( xx_dates(end), 'yyyymmdd'), n_min, 100.*prct ) );
fprintf( 'Writing %s...\n', matfile );
save( matfile, 'cnf', 'opts', 'xx_dates', 'xx_val', 'n_min', 'n_sel', 'prct' );


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
fprintf( '*                         Monte Carlo Validation Toolkit             *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*         RIO (c) VITO/IRCEL 2004-2015                               *\n' );
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
fprintf( '  riomcval [options] <pol> <agg_time> <proxy>\n' );
fprintf(' Where : \n' );
fprintf('    pol ......... : pm10, pm25, no2, o3, so2, nh3 \n' );
fprintf('    agg_time .... : m1, m8, da or 1h  \n' );
fprintf('    proxy ....... : CorineID, CorineID_double_beta, AOD, ... \n' );
fprintf('\n' );
fprintf(' Optional arguments, prefixed by "--" :\n' );
fprintf('    --help                  : this message\n' );
fprintf('    --pct <number> ........ : fraction of station to leave out (in percent, def. 20)\n' );
fprintf('    --n_min <number> ...... : run untill each station was validated n_min times (def. 1)\n' );
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
