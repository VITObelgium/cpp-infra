%OPAQ_UPDATE_OVL_METEODB Updates an OVL mat file with the dates & meteo
%
% Usage:
%   [ h ] = opaq_write_ovl_meteodb( dbname, xx_date_new, xx_meteo_new, settings )
%
% Arguments
%   - fname .......... : filename for the OVL .mat database
%   - xx_date_new .... : column vector of the dates
%   - xx_meteo_new ... : matrix of the meteo parameters, one date entry for
%                        every row, the number of columns gives the
%                        different variables...
%
%   - settings................ : structure with the settings of the new data
%     settings.tzone ......... : timezone of the new data
%     settings.xx_hours ...... : hours for each day in the new data
%     settings.basetime_utc .. : the datenum of the GFS basetime in UTC for this update
%     settings.plist ......... : parameters list, as returned by opaq_extract_meteo
%     settings.make_plot ..... : create a plot ?
%
%
% Bino Maiheu, (c) 2014 VITO

function varargout = opaq_update_ovl_meteodb( dbname, xx_date_new, xx_meteo_new, s  )

yy = load( dbname );

% read xx_hours from file and check whether they are consistent with
% our current configuration
if ~isfield( yy, 'xx_hours' ) || ...
        ~isfield( yy, 'xx_meteo' ) || ...
        ~isfield( yy, 'gfs_base') || ...
        ~isfield( yy, 'tzone')
    error( 'Error: %s: is not a proper meteo file...', dbname );
end

if numel(yy.xx_hours) ~= numel(s.xx_hours)
    error( 'Error: %s: different number of hours per day in file...', dbname );
end

if any( yy.xx_hours - s.xx_hours ) || ~strcmp( yy.tzone, s.tzone )
    error( 'Error: %s: incompatible hours and/or timezone...', dbname );
end

npars     = numel(unique(yy.xx_meteo(:,2)));
plot_cols = floor(sqrt(npars)); % floor of qsrt of number of meteo vars, e.g. 15 -> 3
plot_rows = ceil(npars/plot_cols);

% get the timestep in hours
hr_step  = unique(diff(s.xx_hours));

% Checking the number of meteo variables in the new/old
% file : second column of the yy.xx_meteo and 2nd dimension
% size of the xx_meteo_new
if numel(unique(yy.xx_meteo(:,2))) ~= size(xx_meteo_new,2)
    error( 'Error: %s: different number of meteo variables', dbname );
end

% Check the gfs basetime in the datafile and produce a
% warning if it's earlier than the last update
if ( s.basetime_utc < max(yy.gfs_base) )
    fprintf( '+++ WARNING: attempting to update with earlier forecast than last entry (last: %s), (new: %s)\n', ...
        datestr( max(yy.gfs_base(end)), 'yyyy.mm.dd-HH' ), datestr( s.basetime_utc, 'yyyy.mm.dd-HH') );
end
      
% Append the update time, make a new gfs_base
gfs_base = [ yy.gfs_base; s.basetime_utc ];
                 
% loop over variables and unpack the array as we want to produce some
% timeseries plots
% optimisaiton would be to include this part with the stuff above
% for creating a new file...
xx_meteo = [];
if s.make_plot
    hFig = figure( 'Position', [0 0 1200 800 ],  'Visible', 'off' );
end
                 
% loop over the variables in the meteo file, they should
% match the entries in the second dimension of
% xx_meteo_new
for p=1:npars
                     
    % find this parameter in the old file
    idx_p = find( yy.xx_meteo(:,2)==p);
                     
    % unpack into simple 1D array
    old_meteo = reshape( yy.xx_meteo(idx_p,3:end)', numel(yy.xx_meteo(idx_p,3:end)), 1 );
    old_date  = repmat( yy.xx_meteo(idx_p,1), 1, length(yy.xx_hours) ) + ...
        repmat(datenum( 0,0,0,yy.xx_hours,0,0 )', size(yy.xx_meteo(idx_p,1),1), 1 );
    old_date  = reshape( old_date', numel(old_date), 1 );
                     
    
    if s.make_plot
        % arg bit of hard coding here with the figure...
        subplot( plot_rows, plot_cols, p );
        set(gca, 'FontSize', 8  );
        plot( old_date, old_meteo, 'k', 'LineWidth', 1.5 );
        hold on;
    end
    % update the array
    [ tmp_date, tmp_meteo ] = opaq_update_array( old_date, old_meteo, ...
        xx_date_new', squeeze(xx_meteo_new(:,p)) );
                     
    % now check the edges of the array and fill if needed...
    % we fore sure have a contiguous timeseries provided the first
    % complete upload was contiguous...
    while( get_hour(tmp_date(1)) ~= s.xx_hours(1) )
        tmp_date  = [ tmp_date(1)-datenum(0,0,0,hr_step,0,0); tmp_date ];
        tmp_meteo = [ NaN; tmp_meteo ];
    end
        
    while( get_hour(tmp_date(end)) ~= s.xx_hours(end) )
        tmp_date  = [ tmp_date; tmp_date(end)+datenum(0,0,0,hr_step,0,0) ];
        tmp_meteo = [ tmp_meteo; NaN ];
    end
                           
    if s.make_plot
        % and plot all of the stuff
        plot( tmp_date, tmp_meteo, 'r' );
                     
        % plot the new stuff separately
        new_meteo = squeeze(xx_meteo_new(:,p));
        new_date  = xx_date_new';
        plot( new_date, new_meteo, 'b' );
                     
        xlim( [ min(tmp_date) max(tmp_date) ] );
        datetick( 'x', 'keeplimits', 'keepticks' );
        title( strrep( s.plist(p).short, '_', '\_' ) );
        hold off;
    end
                                          
    nn_meteo  = length(tmp_meteo);
    tmp_meteo = reshape(tmp_meteo,length(s.xx_hours),nn_meteo/length(s.xx_hours))';
    tmp_days  = floor(reshape(tmp_date,length(s.xx_hours),nn_meteo/length(s.xx_hours)))';
        
    % just a safety check...
    if any( any( bsxfun( @minus, tmp_days, tmp_days(:,1) ) ) )
        error( 'opaq_import_gfs : date/time array mismatch, fatal error, contact bino...' );
    end
        
    xx_meteo = [ xx_meteo; tmp_days(:,1) p*ones(length(tmp_days),1) tmp_meteo ];
                                                               
end % end loop over variables list
                    
% return the plot if requested
if nargout > 0 && s.make_plot
    varargout{1} = hFig;
end
             
% save ovl db
xx_hours = yy.xx_hours;
tzone    = yy.tzone;

save( dbname, 'xx_meteo', 'xx_hours', 'tzone', 'gfs_base' );

