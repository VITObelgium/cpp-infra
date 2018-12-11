%RIO_CALCSTATS
% Calculate statistics from the loaded configuration with historic
% database. Several options can be specified by the options structure. 
% Depending on the options specified, the parameters in the
% rio_param/stat_param folder are updated.
%
% rio_calcstats( cnf )
% rio_calcstats( cnf, opt )
%
% opt is a configuration structure containing
%  opt.time_window ... : use this timewindow when recomputing stats
%                        when this field is absent, the whole period 
%                        is used.
%  opt.weekpart ...... : either 'all', 'week', 'weekend'
%  opt.overwrite ..... : overwrite the files : 'yes' / 'no' / 'ask'
%  opt.cutoff ........ : apply cut-off for calculating statistics
%
% See also rio_init, rio_calcspcorr, rio_gettrend
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ xx_avg, xx_std ] = rio_calcstats( cnf, varargin )

xx_avg     = [];
xx_std     = [];

if nargin == 1
    error( 'DEFAULT MODE NOT IMPLEMENTED YET...' );     
elseif nargin == 2
    opt = varargin{1};
    if ~isfield( opt, 'weekpart' ), opt.weekpart  = 'all'; end;
    if ~isfield( opt, 'overwrite' ), opt.overwrite = 'no'; end;
end

if ~cnf.have_db
    fprintf( 'error rio_calcstats:: need to load db first !\n' );
    return;
end

%-- Weekday identifier (1 --> 7)
dw = weekday( cnf.xx_date );

%-- Make date selection
week_i    = find( dw >= 2 & dw <= 6 );
weekend_i = find( dw == 1 | dw == 7 );
if isfield( opt, 'time_window')
    time_widow_i = find( cnf.xx_date >= opt.time_window(1) & ...
        cnf.xx_date <= opt.time_window(2) );
end
    
if strcmp( opt.weekpart, 'week')
    date_i = week_i;
elseif strcmp( opt.weekpart, 'weekend')
    date_i = weekend_i;
elseif strcmp( opt.weekpart, 'all')
    date_i = union(week_i, weekend_i);
end

%-- If required, apply cut on time window...
if isfield( opt, 'time_window' )
    date_i = intersect( time_widow_i , date_i );
end
    
%-- Loop over all stations, calc mean and std dev for pollutant XX...
for i = 1:cnf.nr_st
    st_i = cnf.st_info(i,1);
    %-- Select station events
    stat_i = find(cnf.xx_val(:,1) == st_i);
    index = intersect(date_i, stat_i);
            
    xx_tmp = cnf.xx_val(index, cnf.agg_time_loc );    
    
    % Apply cut off if necessary...
    if isfield( opt, 'cutoff' )
        xx_tmp( xx_tmp > opt.cutoff ) = NaN;
    end
    
    % Add a safety check
    xx_tmp( xx_tmp < 0 ) = NaN;
    
    for at = 1:size(xx_tmp,2)
        xx_tmp_at = xx_tmp(:,at);
        xx_tmp_at = xx_tmp_at(~isnan(xx_tmp_at));
    
        if isempty(xx_tmp_at)
            xx_avg_tmp(at) = NaN;
            xx_std_tmp(at) = NaN;
            fprintf( '+++ rio_calcstats warning: no data found for %s, agg_loc = %d\n', ...
                cnf.st_id{i}, at );
        else
            xx_avg_tmp(at) = mean(xx_tmp_at);
            xx_std_tmp(at) = std(xx_tmp_at);
        end
    end
    
    xx_avg = [xx_avg; [st_i xx_avg_tmp]];
    xx_std = [xx_std; [st_i xx_std_tmp]];
end

%-- overwrite ?
if strcmpi( opt.overwrite, 'ask' )
    opt.overwrite = lower( questdlg( 'Overwrite the RIO long term statistics ?', ...
        'Overwrite parameters ?', 'Yes', 'No', 'No' ) );
end
if strcmpi( opt.overwrite, 'yes' )
    % does the pol_xx folder exist ?        
    if ~exist( fullfile( cnf.paramPath, 'stat_param', '' ), 'dir' )
        rio_log( sprintf( 'creating statistics parameter folder for %s\n', cnf.pol_xx ) );
        mkdir( cnf.paramPath, 'stat_param' );
    end
    
    % timestamp for backups
    tstamp = datestr( now, 30 );
    
    % log this !!
    fprintf( 'rio_calcstats: overwriting stat_param for %s %s agg_time: %s\n', ...
        cnf.pol_xx, opt.weekpart, cnf.at_lb );
    if isfield( opt, 'time_window' )
        rio_log( sprintf( 'rio_calcstats: overwriting stat_param %s, weekpart: %s, agg_time: %s, window: [ %s -> %s ]', ...
            cnf.pol_xx, opt.weekpart, cnf.at_lb, datestr( opt.time_window(1), 30 ), datestr( opt.time_window(2), 30 ) ) );    
    else
        rio_log( sprintf( 'rio_calcstats: overwriting stat_param %s, weekpart: %s, agg_time: %s, window: all data', ...
            cnf.pol_xx, opt.weekpart, cnf.at_lb ) );             
    end

    % ---------------------------------------------------------------------
    % save the avg statistics
    % ---------------------------------------------------------------------
    fname = fullfile( cnf.paramPath, 'stat_param', ...
        sprintf( 'avg_%s_%s_agg_time-%s.mat', cnf.pol_xx, opt.weekpart, cnf.at_lb ) );
    if exist( fname, 'file' )
        bak_file = sprintf( '%s.%s.bak', fname, tstamp );
        movefile( fname, bak_file );
        rio_log( sprintf( ' - backup %s to %s', fname, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', fname ) );
    save( fname, 'xx_avg'); 
    
    % ---------------------------------------------------------------------
    % save the std statistics
    % ---------------------------------------------------------------------
    fname = fullfile( cnf.paramPath, 'stat_param', ...
        sprintf( 'std_%s_%s_agg_time-%s.mat', cnf.pol_xx, opt.weekpart, cnf.at_lb ) );
    if exist( fname, 'file' )
        bak_file = sprintf( '%s.%s.bak', fname, tstamp );
        movefile( fname, bak_file );
        rio_log( sprintf( ' - backup %s to %s', fname, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', fname ) );
    save( fname, 'xx_std'); 
    
end
