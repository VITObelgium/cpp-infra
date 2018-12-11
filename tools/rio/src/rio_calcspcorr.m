%RIO_CALCSPCORR
%  Calculate spatial correlation model from the loaded configuration with 
%  historic database. Several options can be specified by the options 
%  structure. Depending on the options specified, the parameters in the
%  rio_param/spatial_corr folder are updated.
%  
%  rio_calcspcorr( cnf )
%  rio_calcspcorr( cnf, opt )
%
%  opt is a configuration structure containing :
%
%  General options
%   opt.time_window ... : use this timewindow when recomputing stats
%                         when this field is absent, the whole period 
%                         is used.
%   opt.overwrite ..... : overwrite the files : 'yes' / 'no' / 'ask' 
%
%  Spatial correlation options
%   opt.short ......... : det.short range correlation model (def. not)
%   opt.short_range ... : range for this model (def. 20 km)
%   opt.detrend ....... : detrend data before calc. spatial corr (def. true)
%   opt.profile ....... : creates a profile histogram first before fitting
%                         and performs a weighted fit from the profile.
%   opt.prof_binsize .. : binsize for profile, in km
%   opt.corr_model .... : 'exp', 'expm', exponential or modified
%                         exponential
%   opt.make_plot ..... : make a plot of the spatial correlation model
%
% Changelog
%  - 2011.01.18 BM. : - leave out the log-transform, we're looking for
%                       correlations anyway, so should not matter.
%                     - also there is still some non-elegant stuff in here
%                       concerning the time window and the detrending...
%
% See also rio_init, rio_calcstats, rio_gettrend, rio_distmat,
%          rio_spcorrmat, rio_fitspcorr
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ p, p_short ] = rio_calcspcorr( cnf, varargin )

if nargin == 1   
    error( 'DEFAULT MODE NOT IMPLEMENTED YET...' );     
elseif nargin == 2
    opt = varargin{1};
    if ~isfield( opt, 'overwrite' ),   opt.overwrite = 'no'; end;
    if ~isfield( opt, 'short' ),       opt.short = false; end;
    if ~isfield( opt, 'short_range' ), opt.short_range = 20.; end;   
    if ~isfield( opt, 'detrend' ),     opt.detrend = true; end;
end

if ~cnf.have_db
    fprintf( 'error rio_calcspcorr:: need to load db first !\n' );
    return;
end

% Only use these station indices, can be integrated in the cnf later on
% use all of them for now
st_list = 1:cnf.nr_st;
st_idx  = [];
for i=1:length( st_list )
    id = find( cnf.st_info(:,1) == st_list(i) );
    if isscalar( id ),
        st_idx = [ st_idx ; id ];
    end
end

% -- Select a timewindow, no week/weekend distinction in spatial
%    correlations
%-- If required, apply cut on time window...
if isfield( opt, 'time_window' )   
    date_i = find( cnf.xx_date >= opt.time_window(1) & ...
        cnf.xx_date <= opt.time_window(2) );  
    xx_date = cnf.xx_date( date_i );
else
    error( 'Currently we need a timewindow for this routine !!!' );
end
dates   = unique( xx_date );

% -- Compute stations distance matrix for station selection, routine needs
%    a set of [ x y ] coordinates
r_mat = rio_distmat( [ cnf.st_info( st_idx, 2 ) cnf.st_info( st_idx, 3 ) ] );
r_vec = mat2vec( r_mat );

% -- Prepare plot ?
if opt.make_plot
    figure;
end

% -- what aggregation time do we have ?
if cnf.agg_time < 4, at_loop = 1; else at_loop = 24; end
for at = 1:at_loop
    
    if opt.detrend
        xx_val  = []; % prepare the values
        xx_date = []; % overwrite the dates again, we'll get them from the detrending
        
        fprintf( 'rio_calcspcorr: detrending, at=%d...\n', at );
        % -- If opt.detrend we need to have the long term averages loaded, we
        %    load the 'all' version since for the spatial correlations, we don't
        %    discriminate between week/weekend as we use temporal correlation and
        %    the distinction would mess this up !, but we have to loop over
        %    the dates as the librio implementation only supports
        %    de/re-trending on a single day... maybe a todo item !!
        cnf     = rio_updatepars( cnf, 'all', at );          
        
        for k=1:length( dates )         
            
            % need to add the hour to the date
            the_date = dates(k) + datenum( 0, 0, 0, at-1, 0, 0 );
            
            [ st_info_tmp, xx ] = rio_dblookup( cnf, the_date );
            xx_detr = rio_detrend( cnf, st_info_tmp, xx );
            
            xx_val  = [ xx_val;  xx_detr ];
            xx_date = [ xx_date; dates(k)*ones( size(xx_detr,1), 1) ];
            
        end
    else
        % Just set the data from the cnf using the correct time window...
        if isfield( opt, 'time_window' )
            xx_val = cnf.xx_val( date_i, [ 1 cnf.agg_time_loc(at) ] );
        else
            xx_val = cnf.xx_val( :, [ 1 cnf.agg_time_loc(at) ] );
        end
    end
    
    % log-transform ? leave out for now... not sure what the purpose of
    % this is, we are looking for correlations anyway...
    
    % Correlation matrix
    if isfield( opt, 'time_window' ) 
        c_mat = rio_spcorrmat( st_idx, xx_date, xx_val );
    else
        error( 'Not implemented yet, please define a time_window...\n' );
    end
    
    % Transform the spatial correlation matrix to a vector
    c_vec = mat2vec( c_mat );
   
    % Fit the spatial correlation model
    [ p(at,:), p_short(at,:), model ] = rio_fitspcorr( r_vec, c_vec, opt );   
    
    % Plot the model
    if opt.make_plot
        if cnf.agg_time == 4, subplot( 6, 4, at ); end;
        plot( r_vec, c_vec, '.' );
        hold on;
        r_tmp = 0.:1.:max(r_vec)+opt.prof_binsize;
        plot( r_tmp, model( p(at,:), r_tmp ), 'k' );
        if opt.short
            r_tmp = 0.:1.:opt.short_range;
            plot( r_tmp, polyval( p_short(at,:), r_tmp ), 'k' );
        end
        xlabel( 'r [km]' );
        if cnf.agg_time == 4
            ylabel( sprintf( '\\rho - %02dh', at-1 ) );
        else
            ylabel( sprintf( '\\rho - %s', cnf.at_lb ) );
        end
        set( gca, 'ylim', [ 0. 1. ] );
        hold off;
    end    
end

% -- Write out the parameters
if strcmpi( opt.overwrite, 'ask' )
    opt.overwrite = lower( questdlg( 'Overwrite the RIO spatial correlation parameters ?', ...
        'Overwrite parameters ?', 'Yes', 'No', 'No' ) );    
end
if strcmpi( opt.overwrite, 'yes' )
    % does the pol_xx folder exist ?
    if ~exist( fullfile( cnf.paramPath, 'spatial_corr', '' ), 'dir' )
        rio_log( sprintf( 'creating spatial_corr parameter folder for %s\n', cnf.pol_xx ) );
        mkdir( cnf.paramPath, 'spatial_corr' );
    end
    
    % timestamp for backups
    tstamp = datestr( now, 30 );
    
    % log this !!
    fprintf( 'rio_calcspcorr: overwriting spatial_corr parameters for %s agg_time: %s\n', ...
        cnf.pol_xx, cnf.at_lb );
    rio_log( sprintf( 'rio_calcspcorr: overwriting spatial_corr for %s agg_time: %s', ...
        cnf.pol_xx, cnf.at_lb ) );    
       
    % set additional label
    if ~opt.detrend
        gis_type = 'no_detr';
    else
        gis_type = cnf.gis_type;
    end
    
    % ---------------------------------------------------------------------
    % p
    % ---------------------------------------------------------------------
    trend_file = fullfile( cnf.paramPath, 'spatial_corr', ...
        sprintf( 'p_long_%s_%s_agg_time-%s.mat',  cnf.pol_xx, gis_type, cnf.at_lb ) );
    if exist( trend_file, 'file' )
        bak_file = sprintf( '%s.%s.bak', trend_file, tstamp );
        movefile( trend_file, bak_file );
        rio_log( sprintf( ' - backup %s to %s', trend_file, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', trend_file ) );
    save( trend_file, 'p');
    
    % ---------------------------------------------------------------------
    % p_short
    % ---------------------------------------------------------------------
    trend_file = fullfile( cnf.paramPath, 'spatial_corr', ...
        sprintf( 'p_short_%s_%s_agg_time-%s.mat',  cnf.pol_xx, gis_type, cnf.at_lb ) );
    if exist( trend_file, 'file' )
        bak_file = sprintf( '%s.%s.bak', trend_file, tstamp );
        movefile( trend_file, bak_file );
        rio_log( sprintf( ' - backup %s to %s', trend_file, bak_file  ) );
    end
    rio_log( sprintf( ' - save %s', trend_file ) );
    save( trend_file, 'p_short');
end



% Helper routine to reshape the distance matrix and covariance matrix to 
% vectors, note these are symmetric around the diagonal, so only need to 
% have one half plus check for NaN !!
function v = mat2vec( m )
n = size( m, 1 );
v = reshape( m, 1, n*n )';




