%riopp Post processing tool
%
% Usage : see riopp --help
%
% Developed by Bino Maiheu, (c) VITO 2010-2012
%
% Examples : 
%  riopp --output [txt,hdf5,hdf5_append]
%        [--mc]    
%        rio_output_file1.h5
%
% Changelog
% - Removed temporarily the flexibility for postprocessing less than a year
%   now a full year needs to be postprocessed
% - No longer support the txt files...
%
% RIO (c) VITO/IRCEL 2004-2015


function riopp( varargin )
tic;

%% Configuration
argopts = [ struct( 'name', 'start', 'default', -1, 'cast', @(x)datenum(x,31)), ...    
    struct( 'name', 'stop', 'default', -1, 'cast', @(x)datenum(x,31)), ...     
    struct( 'name', 'mc', 'default', false, 'cast', NaN ), ...
    struct( 'name', 'output', 'default', 'txt', 'cast', @(x)(x) ), ...
    struct( 'name', 'help', 'default', false, 'cast', NaN ) ]; % just a switch

% Parse and some initial checks
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;
if ~( strcmpi( opts.output, 'hdf5' ) || strcmpi( opts.output, 'txt' ) ||...
    strcmpi( opts.output, 'hdf5_append' ) )
    error( 'Unknown output mode : %s, please select [hdf5/txt/both]', opts.output );
end

%-- Welcome!
print_header;
fprintf( '\n' );
fprintf( '                                               (((\n');
fprintf( '                                              (. .)\n');
fprintf( '                                            <(( v ))>\n');
fprintf( '-----------------------------------------------m-m--------------------\n' );
fprintf( ' Initialisation...\n' );
fprintf( '----------------------------------------------------------------------\n' );


% assume the user enters a hdf5 file to read, post process that...
hdf5_input  = args{1};
    
if ~exist( hdf5_input, 'file' )
    error( 'No such file : %s\n', hdf5_input );
else
    fprintf( 'Processing %s...\n', hdf5_input );
end

% get version 
rio_fmt_version = cell2mat( h5readatt( hdf5_input, '/', 'output_format' ) );
if ~strcmp( rio_fmt_version, '1.1' )
    error( 'Postprocessing only works for output format 1.1' )
end

% read missing value from HDF5
pol         = cell2mat(h5readatt( hdf5_input, '/', 'pollutant' ));
agg_timestr = cell2mat(h5readatt( hdf5_input, '/', 'agg_time' ));
ipol_mode   = cell2mat(h5readatt( hdf5_input, '/', 'ipol_mode' ));
grid_type   = cell2mat(h5readatt( hdf5_input, '/', 'grid_type' ));
at_lb       = agg_timestr;
missing_value = -9999.;
x_gr = double( h5read( hdf5_input, '/grid/x' ) );
y_gr = double( h5read( hdf5_input, '/grid/y' ) );
grid_info     = [ [1:size(x_gr,1)]' x_gr(:) y_gr(:) ]; % ID, X, Y
x_st = double( h5read( hdf5_input, '/stations/x' ) );
y_st = double( h5read( hdf5_input, '/stations/y' ) );
st_info       = [ [1:size(x_st,1)]' x_st(:) y_st(:) ]; % ID, X, Y

fprintf( 'Pollutant ............ : %s\n', pol );
fprintf( 'Aggregation .......... : %s\n', agg_timestr );
fprintf( 'Interpolation mode ... : %s\n', ipol_mode );
fprintf( 'Grid type ............ : %s\n', grid_type );


%% Postprocess the file  
    
% Importint the data
xx_dates = [];
timestep = datenum( 0, 0, 1, 0, 0, 0 );

fprintf( 'Importing gridded concentrations...\n' );    
grid_C  =  double( hdf5read( hdf5_input, '/grid/value' )' ) ;
fprintf( 'Importing gridded concentrations uncertainties...\n' );
grid_U  = double( hdf5read( hdf5_input, '/grid/error' )' );

fprintf( 'Importing measured concentrations...\n' );
st_C    = double( hdf5read( hdf5_input, '/stations/value' )' );

    
% construct xx_dates
dd = double( hdf5read( hdf5_input, '/time/day') );
mn = double( hdf5read( hdf5_input, '/time/month') );
yr = double( hdf5read( hdf5_input, '/time/year') );

if strcmp( agg_timestr, '1h')
    hr = double( hdf5read( hdf5_input, '/time/hour') );
    t1 = datenum( yr(1), mn(1), dd(1), hr(1)-1, 0, 0 );
    t2 = datenum( yr(end), mn(end), dd(end), hr(end)-1, 59, 59 );
else
    t1 = datenum( yr(1), mn(1), dd(1), 0, 0, 0 );
    t2 = datenum( yr(end), mn(end), dd(end), 0, 0, 0 );
end
    
xx_dates = t1:timestep:t2;
    
% Set NaN
grid_C( grid_C == missing_value ) = NaN;
grid_U( grid_U == missing_value ) = NaN;
st_C( st_C == missing_value )     = NaN;

fprintf( 'Postprocessing request from %s to %s\n', datestr( xx_dates(1) ), datestr( xx_dates(end) ) );  

% EU norm for yearly averages
switch( pol )
    case 'pm10'
        eu_norm_ya = 40.;
    case 'so2'
        eu_norm_ya = 20.;
    case 'no2'
        eu_norm_ya = 40.;  
    case 'nh3'
        eu_norm_ya =  8.;  % norm for NH3 yearly average
    otherwise
        eu_norm_ya = -1;    
end
fprintf( 'EU norm for yearly %s average : %.1f µg/m3\n', pol, eu_norm_ya );


% EU exceedance limits for the current aggregation time and pollutant
[ eu_limit, eu_max_exceed ] = rio_exceedlimits( agg_timestr, pol );

fprintf( 'Finished initialisation in %f sec.\n', toc );		
fprintf( '\n' );
%% ------------------------------------------------------------------------
%  Do the postprocessing...
fprintf( '                                                           (((\n');
fprintf( '                                                        \\ (. .) /\n');
fprintf( '                                                         (( v ))\n');
fprintf( '-----------------------------------------------------------m-m--------\n' );
fprintf( ' Postprocessing...\n');
fprintf( '----------------------------------------------------------------------\n' );
tic;

%-- Covariance correction factor
fcov = rio_calcfcov( grid_C );
fprintf( 'Covariance correction factor : %f\n', fcov );

% Compute ordinary grid/station averages and uncertainties...
[ grid_avg, st_avg ] = riopp_avg( grid_C, grid_U, st_C, eu_norm_ya, fcov );

% Compute number of exceedances and exceedance probability...
[ grid_noe, st_noe ]  = riopp_exceed( grid_C, st_C, grid_avg(:,3), eu_limit, eu_max_exceed );

fprintf( 'Finished post production in %f sec.\n', toc );	

%% --------------------------------
% make some plots
rio_map( grid_avg(:,2), 'title', sprintf( '%s', pol )  );
saveas( gcf, sprintf( '%s_year_avg.png', pol ), 'png' );
close gcf;

%% -----------------------------------------------------------------------
%  Now output the final results to current dir...
%
% set missing value
grid_avg( isnan( grid_avg ) ) = missing_value;
st_avg( isnan( st_avg ) ) = missing_value;

grid_noe( isnan( grid_noe ) ) = missing_value;
st_noe( isnan( st_noe ) ) = missing_value;


start_date = xx_dates(1);
stop_date  = xx_dates(end);

switch opts.output
    case 'hdf5'
        fname = sprintf( 'riopp_%s_%s_%s_%s_%s-%s.h5', pol, ipol_mode, ...
            grid_type, agg_timestr, datestr( start_date, 29 ), datestr( stop_date, 29 ) );
        
        riopp_outputh5( fname, 'overwrite', agg_timestr, grid_avg, st_avg, grid_noe, st_noe, ...
            eu_norm_ya, eu_limit, eu_max_exceed );
        
        % apparently a new h5 function exists in the new matlab :)
        h5writeatt( fname, '/postprocessing', 'covariance_factor', fcov );
        
    case 'hdf5_append'
        riopp_outputh5( hdf5_input, 'append', agg_timestr, grid_avg, st_avg, grid_noe, st_noe, ...
            eu_norm_ya, eu_limit, eu_max_exceed  );
        
        % apparently a new h5 function exists in the new matlab :)
        h5writeatt( hdf5_input, '/postprocessing', 'covariance_factor', fcov );
        
    case 'txt'   
        fbase_grid = sprintf( '%s_%s_%s_%s-%s-%s_postproc.txt', pol, ipol_mode, ...
            grid_type, datestr( start_date, 29 ), datestr( stop_date, 29 ), ...
            at_lb );
        fbase_stat = sprintf( '%s_%s-%s-%s_stations_postproc.txt', pol, ...
            datestr( start_date, 29 ), datestr( stop_date, 29 ), agg_timestr );
        
        % -- Write the ascii grid
        fprintf( 'Writing %s\n', fbase_grid );
        fid = fopen( fbase_grid, 'wt');
        fprintf(fid, 'ID\tX\tY' );
        fmt = '%d\t%d\t%d';
        gid = 1:3;

        
        fprintf( fid, '\tavg\tavg_err\tavg_pex\tnoe\tnoe_low\tnoe_upp\tpex' );
        fmt = [ fmt '\t%.2f\t%.2f\t%.2f\t%d\t%d\t%d\t%.2f' ];
        if length( eu_limit ) > 1
            fprintf( fid, '\tda_noe\tda_noe_low\tda_noe_upp\tda_pex\n' );
            fmt = [ fmt '\t%d\t%d\t%d\t%.2f\n' ];
            fprintf( fid, fmt, ...
                [ grid_info(:, gid) grid_avg(:,[ 2 3 5] ) grid_noe(:,[ 1 2 3 4 5 6 7 8 ]) ]' );
        else
            fprintf( fid, '\n' );
            fmt = [ fmt '\n' ];
            fprintf( fid, fmt, ...
                [ grid_info(:,gid) grid_avg(:,[ 2 3 5] ) grid_noe(:,[ 1 2 3 4 ]) ]' );
        end
        fclose( fid );
        
        % -- Write the station file -----------------------------------------------
        fprintf( 'Writing %s\n', fbase_stat );
        fid = fopen( fbase_stat, 'wt');
        fprintf(fid, 'ID\tX\tY' );
        fmt = '%d\t%d\t%d';
        gid = [1:3];
        
        fprintf( fid, '\tavg\tnoe' );
        fmt = [ fmt '\t%.2f\t%d' ];
        if length( eu_limit ) > 1
            fprintf( fid, '\tda_noe\n' );
            fmt = [ fmt '\t%d;\n' ];
            fprintf( fid, fmt, ...
                [ st_info(:, gid) st_avg(:,2) st_noe(:,[1 2]) ]' );
        else
            fprintf( fid, '\n' );
            fmt = [ fmt '\n' ];
            fprintf( fid, fmt, ...
                [ st_info(:, gid) st_avg(:,2) st_noe(:,1) ]' );
        end
        
        fclose(fid);

    otherwise
        error( 'oops, should not happen...' );
end





fprintf( 'Finished post processing in %f sec.\n', toc );
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
fprintf( '  riopp [options] <hdf5files>\n' );
fprintf(' Optional arguments, prefixed by "--" :\n' );
fprintf('    --help                          : this message\n' );
fprintf('    --mc                            : Switch on Monte Carlo routines\n' );
fprintf('    --output [txt,hdf5,hdf5_append] : Output format (def: txt)\n' );
fprintf('\n' );
fprintf('RIO (c) VITO/IRCEL 2004-2012\n');
fprintf('Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu\n');

function print_header
v = rio_version;
fprintf( '                                                    (((\n');
fprintf( '                                                   (. .)\n');
fprintf( '                                                  (( v ))\n');
fprintf( '************************************************ ---m-m--- ***********\n' );
fprintf( '*                         Welcome to the RIO                         *\n' );
fprintf( '*                       Post-processing Tookit                       *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*         RIO (c) VITO/IRCEL 2004-2012                               *\n' );
fprintf( '*         Jef Hooybergs, Stijn Janssen, Frans Fierens                *\n' );
fprintf( '*         Nele Veldeman, Bino Maiheu                                 *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*   Contact: bino.maiheu@vito.be                                     *\n' );
fprintf( '*                                                                    *\n' );
fprintf( '*   Library version : %d.%d                                            *\n', v.major, v.minor );
fprintf( '**********************************************************************\n' );

function riopp_outputh5( fname, mode, at_lb, grid_avg, st_avg, grid_noe, st_noe, ...
    eu_norm_ya, eu_limit, eu_max_exceed )

fprintf( 'Writing output to %s...\n', fname );

hdf5write( fname, '/postprocessing/grid/year_avg',        grid_avg(:,2), 'WriteMode', mode);
riopp_setattr( fname, '/postprocessing/grid/year_avg', 'description', 'Yearly average concentration' );
riopp_setattr( fname, '/postprocessing/grid/year_avg', 'units', 'µg/m3' );

hdf5write( fname, '/postprocessing/grid/year_avg_err',     grid_avg(:,3), 'WriteMode', 'append' );
riopp_setattr( fname, '/postprocessing/grid/year_avg_err', 'description', 'Standard Error on yearly average concentration' );
riopp_setattr( fname, '/postprocessing/grid/year_avg_err', 'units', 'µg/m3' );

hdf5write( fname, '/postprocessing/grid/year_std',        grid_avg(:,4), 'WriteMode', 'append' );
riopp_setattr( fname, '/postprocessing/grid/year_std', 'description', 'Standard Deviation of concentrations' );
riopp_setattr( fname, '/postprocessing/grid/year_std', 'units', 'µg/m3' );

hdf5write( fname, '/postprocessing/grid/year_avg_exceedance_probability', grid_avg(:,5), 'WriteMode', 'append' );
riopp_setattr( fname, '/postprocessing/grid/year_avg_exceedance_probability', 'description', 'Exceedance probability of EU norm on yearly average' );
riopp_setattr( fname, '/postprocessing/grid/year_avg_exceedance_probability', 'units', '%' );
riopp_setattr( fname, '/postprocessing/grid/year_avg_exceedance_probability', 'EU_norm_yearly_aveage',  eu_norm_ya );

field = sprintf( '/postprocessing/grid/number_of_exceedances_%s', at_lb );
hdf5write( fname, field, grid_noe(:,1), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', sprintf( 'Number of exceedances of %s threshold value', at_lb ) );
riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(1) );
riopp_setattr( fname, field, 'units', '1' );

field =  sprintf( '/postprocessing/grid/number_of_exceedances_%s_lo', at_lb );
hdf5write( fname, field, grid_noe(:,2), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', sprintf( 'Lower limit for number of exceedances of %s threshold value', at_lb ) );
riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(1) );
riopp_setattr( fname, field, 'units', '1' );

field =  sprintf( '/postprocessing/grid/number_of_exceedances_%s_hi', at_lb );
hdf5write( fname, sprintf( '/postprocessing/grid/number_of_exceedances_%s_hi', at_lb ), grid_noe(:,3), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', sprintf( 'Upper limit for number of exceedances of %s threshold value', at_lb ) );
riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(1) );
riopp_setattr( fname, field, 'units', '1' );

field =  sprintf( '/postprocessing/grid/number_of_exceedances_%s_exceedance_probability', at_lb );
hdf5write( fname, field, grid_noe(:,4), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', sprintf( 'Probability of exceeding EU norm of maximum allowed exceedances of the %s threshold value', at_lb ) );
riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(1) );
riopp_setattr( fname, field, 'EU_norm_max_exceedances_of_threshold', eu_max_exceed(1) );
riopp_setattr( fname, field, 'units', '%' );


if length( eu_limit ) == 2
    field = '/postprocessing/grid/number_of_exceedances_da';
    hdf5write( fname, field, grid_noe(:,5), 'WriteMode', 'append' );
    riopp_setattr( fname, field, 'description', 'Number of exceedances of da threshold value' );
    riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(2) );
    riopp_setattr( fname, field, 'units', '1' );

    field = '/postprocessing/grid/number_of_exceedances_da_lo';
    hdf5write( fname, field, grid_noe(:,6), 'WriteMode', 'append' );
    riopp_setattr( fname, field, 'description', 'Lower limit for number of exceedances of da threshold value' );
    riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(2) );
    riopp_setattr( fname, field, 'units', '1' );

    field = '/postprocessing/grid/number_of_exceedances_da_hi';
    hdf5write( fname, field, grid_noe(:,7), 'WriteMode', 'append' );
    riopp_setattr( fname, field, 'description', 'Upper limit for number of exceedances of da threshold value' );
    riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(2) );
    riopp_setattr( fname, field, 'units', '1' );

    field = '/postprocessing/grid/number_of_exceedances_da_exceedance_probability';
    hdf5write( fname, field, grid_noe(:,8), 'WriteMode', 'append' );
    riopp_setattr( fname, field, 'description', 'Probability of exceeding EU norm of maximum allowed exceedances of the da threshold value' );
    riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(2) );
    riopp_setattr( fname, field, 'EU_norm_max_exceedances_of_threshold', eu_max_exceed(2) );
    riopp_setattr( fname, field, 'units', '%' );
end

% Writing station data...
field = '/postprocessing/stations/year_avg';
hdf5write( fname, field, st_avg(:,2), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', 'Yearly average concentration' );
riopp_setattr( fname, field, 'units', 'µg/m3' );

field = '/postprocessing/stations/year_std';
hdf5write( fname, field, st_avg(:,3), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', 'Concentration standard deviation' );
riopp_setattr( fname, field, 'units', 'µg/m3' );

field = sprintf( '/postprocessing/stations/number_of_exceedances_%s', at_lb );
hdf5write( fname, field, st_noe(:,1), 'WriteMode', 'append' );
riopp_setattr( fname, field, 'description', sprintf( 'Number of exceedances of %s threshold value', at_lb ) );
riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(1) );
riopp_setattr( fname, field, 'units', '1' );

if length( eu_limit ) == 2
    field = '/postprocessing/stations/number_of_exceedances_da';
    hdf5write( fname, field, st_noe(:,2), 'WriteMode', 'append' );
    riopp_setattr( fname, field, 'description', 'Number of exceedances of da threshold value' );
    riopp_setattr( fname, field, 'EU_norm_exceedance_threshold', eu_limit(2) );
    riopp_setattr( fname, field, 'units', '1' );
end

% Write general attributes
riopp_setattr( fname, '/postprocessing/', 'description', 'riopp post-processing results' );
riopp_setattr( fname, '/postprocessing/', 'production_time', datestr(now,31) );

% Apparently in the newer matlab, a h5writeattr exists, so this should be
% redundant... let's leave it in for now...
function riopp_setattr( fname, loc, name, value )
details.Name       = name;
details.AttachedTo = loc;
if strcmp( loc(end), '/' )
    details.AttachType = 'group';
else
    details.AttachType = 'dataset';
end
hdf5write( fname, details, value, 'WriteMode', 'append' );
