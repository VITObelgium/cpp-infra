%OPAQ_IMPORT_GFS Import GFS data into OPAQ (OVL)
%
% Usage:
%   opaq_import_gfs [options] LON1,LAT1;LON2,LAT2 YYYYMMDD HH
%
% Available options:
%   --help ................ : this message
%   --repo <path> ......... : path to repository, excl year (def. gfs)
%   --domain <name> ....... : name for the domain (def. china)  
%   --tzone <shift> ....... : conversion from UTC to local time zone
%                             default 0 : no conversion. for conversion to
%                             CST use --tzone 8 (i.e. add 8 hours to UTC)
%   --mode <name> ......... : select output/import mode : 
%                              'ovldb'  : make ovl matlab db files (def.)
%                              'ovlasc' : write ovl ascii files
%   --output <path> ....... : output folder
%   --plot ................ : make a plot, stored in output folder
%
% Bino Maiheu, (c) VITO 2014 
% Contact: bino.maiheu@vito.be

function opaq_import_gfs( varargin )

% -- Define command line options...
argopts = [ ...
    struct( 'name', 'domain', 'default', 'china', 'cast', @(x)(x)), ...  
    struct( 'name', 'repo',   'default', 'gfs', 'cast', @(x)(x)), ...
    struct( 'name', 'tzone',  'default', 0, 'cast', @(x)(str2double(x))), ...    
    struct( 'name', 'output', 'default', '.', 'cast', @(x)(x) ), ...
    struct( 'name', 'mode',   'default', 'ovldb', 'cast', @(x)(x) ), ...
    struct( 'name', 'plot',   'default', false, 'cast', NaN ), ...
    struct( 'name', 'help',   'default', false, 'cast', NaN ) ]; % just a switch

% -- Parse and some initial checks!
[ opts, args ] = getopt_cast( argopts, '--', varargin );
if opts.help, print_usage; return; end;
if length(args) < 3, error( 'Error in arguments, try --help.' ); end;

% -- Parse the coordinates...
coords     = parse_coords( args{1} );
n_coords   = size(coords,1);

for i=1:n_coords
   fprintf( 'Requested lon=%.2f, lat=%.2f\n', coords(i,1), coords(i,2) ); 
end

% -- Filename to import...
%    fc_base (allow both YYYY-MM-DD and YYYYMMDD) & fc basetime
try
    fc_base = floor(datenum( args{2} ));
catch ME
    fc_base = floor(datenum( args{2}, 'yyyymmdd' ));
end
fc_time  = str2double( args{3} );
if ~(( fc_time == 0 ) || ( fc_time == 6 ) ||( fc_time == 12 ) || ...
        ( fc_time == 18 ) )
    error( 'Error in GFS base time for forecast (either 0,6,12 or 18)' );
end
ncfile = fullfile( opts.repo, sprintf( '%04d', get_year(fc_base) ), ...
    sprintf( 'gfs.%s.%s.%02d.nc', opts.domain, datestr( fc_base, 'yyyymmdd'), fc_time  ) );
if ~exist( ncfile, 'file' )
    error( 'No such file: %s', ncfile )
end
fprintf( 'Importing %s ...\n', ncfile );


% -- extract the OPAQ meteo from the file
try
    fprintf( 'Processing %s...\n', ncfile );
    [ xx_date_new, xx_meteo_new, plist ] = opaq_extract_meteo( ncfile, coords );
catch MEx
    error( 'unable to extract meteo from file: %s...\n', ncfile );    
end

% -- Get the basetime_utc from the first entry & filename & check
basetime_utc = xx_date_new(1);
if abs( basetime_utc - ( fc_base+datenum(0,0,0,fc_time,0,0) ) ) > 1e-6
    error( 'First date in file is not basetime... strange...' );
end


% Apply timeshift if needed...
tzone = 'UTC';
if opts.tzone ~= 0,
    if opts.tzone < 0, 
        tzone = sprintf( 'UTC -%02d:00', abs(opts.tzone) );
    else
        tzone = sprintf( 'UTC +%02d:00', abs(opts.tzone) );
    end,
    
    fprintf( 'Converting timestamps to %s\n', tzone );
    
    xx_date_new = xx_date_new + datenum(0,0,0,opts.tzone,0,0);
end

% extract the hours and create an hours vector...
dv = datevec(xx_date_new);
xx_hours = sort(unique(dv(:,4)));

% -- update the databases...
switch lower(opts.mode)
    
    % -- updating the ovl database    
    case 'ovldb'
        for k=1:n_coords
            fprintf( 'Exporting pars for lon=%.2f, lat=%.2f\n', coords(k,1), coords(k,2) );
            dbname = fullfile( opts.output, sprintf( 'GFS_%.0f_%.0f.mat', 10*coords(k,1), 10*coords(k,2) ) );
            
            if ~exist( dbname, 'file' )    
                fprintf( 'Creating %s...\n', dbname );
                % creating a new file with contents of the GFS forecast
                % here we also explicitely need to mention the updateLog 
                % parameter with the utc basetime of this particular GFS
                % file...
                opaq_write_ovl_meteodb( dbname, xx_date_new, squeeze(xx_meteo_new(k,:,:))', ...
                    'timeZone', tzone, 'gfsBase', basetime_utc );                
                
            else
                % updating an existing file
                fprintf( 'Updating %s...\n', dbname );
                
                % create the settings structure
                s = struct( ...
                    'tzone', tzone, ...
                    'xx_hours', xx_hours, ...
                    'basetime_utc', basetime_utc, ...
                    'plist', plist, ...
                    'make_plot', opts.plot );
                
                h = opaq_update_ovl_meteodb( dbname, xx_date_new, squeeze(xx_meteo_new(k,:,:))', s );
                
                if opts.plot
                    % save plot
                    plfile = fullfile( opts.output, sprintf( 'opaq_gfs_import_%.0f_%.0f-%s.png', ...
                        10*coords(k,1), 10*coords(k,2), datestr( basetime_utc, 'yyyymmdd.HH' ) ) );
                    fprintf( 'Saving %s...\n', plfile );
                    saveas( h, plfile, 'png' );
                    close( h );
                end
   
                %                                 
%                 yy = load( dbname );
%       
%                 % read xx_hours from file and check whether they are consistent with
%                 % our current configuration
%                 if ~isfield( yy, 'xx_hours' ) || ...
%                         ~isfield( yy, 'xx_meteo' ) || ...
%                         ~isfield( yy, 'gfs_base') || ...
%                         ~isfield( yy, 'tzone')
%                     error( 'Error: %s: is not a proper meteo file...', dbname );
%                 end
%                 
%                 if numel(yy.xx_hours) ~= numel(xx_hours)
%                     error( 'Error: %s: different number of hours per day in file...', dbname );
%                 end
%                 
%                 if any( yy.xx_hours - xx_hours ) || ~strcmp( yy.tzone, tzone )
%                     error( 'Error: %s: incompatible hours and/or timezone...', dbname );
%                 end
%                    
%                 npars     = numel(unique(yy.xx_meteo(:,2)));
%                 plot_cols = floor(sqrt(npars)); % floor of qsrt of number of meteo vars, e.g. 15 -> 3
%                 plot_rows = ceil(npars/plot_cols);
%                 
%                 % get the timestep in hours
%                 hr_step  = unique(diff(xx_hours));
%                 
%                 % Checking the number of meteo variables in the new/old
%                 % file : second column of the yy.xx_meteo and 2nd dimension
%                 % size of the xx_meteo_new
%                 if numel(unique(yy.xx_meteo(:,2))) ~= size(xx_meteo_new,2)
%                     error( 'Error: %s: different number of meteo variables', dbname );
%                 end
%                                 
%                  % Check the gfs basetime in the datafile and produce a
%                  % warning if it's earlier than the last update                
%                  if ( basetime_utc < max(yy.gfs_base) )
%                      fprintf( '+++ WARNING: attempting to update with earlier forecast than last entry (last: %s), (new: %s)\n', ...
%                          datestr( max(yy.gfs_base(end)), 'yyyy.mm.dd-HH' ), datestr( basetime_utc, 'yyyy.mm.dd-HH') );
%                  end
%       
%                  % Append the update time, make a new gfs_base
%                  gfs_base = [ yy.gfs_base; basetime_utc ];
%                  
%                  % loop over variables and unpack the array as we want to produce some
%                  % timeseries plots
%                  % optimisaiton would be to include this part with the stuff above
%                  % for creating a new file...
%                  xx_meteo = [];
%                  if opts.plot
%                      hFig = figure( 'Position', [0 0 1200 800 ],  'Visible', 'off' );
%                  end
%                  
%                  % loop over the variables in the meteo file, they should
%                  % match the entries in the second dimension of
%                  % xx_meteo_new
%                  for p=1:npars
%                      
%                      % find this parameter in the old file
%                      idx_p = find( yy.xx_meteo(:,2)==p);
%                      
%                      % unpack into simple 1D array
%                      old_meteo = reshape( yy.xx_meteo(idx_p,3:end)', numel(yy.xx_meteo(idx_p,3:end)), 1 );
%                      old_date  = repmat( yy.xx_meteo(idx_p,1), 1, length(yy.xx_hours) ) + ...
%                          repmat(datenum( 0,0,0,yy.xx_hours,0,0 )', size(yy.xx_meteo(idx_p,1),1), 1 );
%                      old_date  = reshape( old_date', numel(old_date), 1 );
%                      
%         
%                      if opts.plot
%                          % arg bit of hard coding here with the figure...
%                          subplot( plot_rows, plot_cols, p );
%                          set(gca, 'FontSize', 8 );
%                          plot( old_date, old_meteo, 'k', 'LineWidth', 1.5 );
%                          hold on;
%                      end
%                      % update the array                  
%                      [ tmp_date, tmp_meteo ] = opaq_update_array( old_date, old_meteo, ...
%                          xx_date_new', squeeze(xx_meteo_new(k,p,:)) );
% 
%                      
%                      % now check the edges of the array and fill if needed...
%                      % we fore sure have a contiguous timeseries provided the first
%                      % complete upload was contiguous...                        
%                      while( get_hour(tmp_date(1)) ~= xx_hours(1) )
%                          tmp_date  = [ tmp_date(1)-datenum(0,0,0,hr_step,0,0); tmp_date ];
%                          tmp_meteo = [ NaN; tmp_meteo ];
%                      end
%         
%                      while( get_hour(tmp_date(end)) ~= xx_hours(end) )
%                          tmp_date  = [ tmp_date; tmp_date(end)+datenum(0,0,0,hr_step,0,0) ];
%                          tmp_meteo = [ tmp_meteo; NaN ];
%                      end
%                            
%                      if opts.plot
%                          % and plot all of the stuff
%                          plot( tmp_date, tmp_meteo, 'r' );
%                      
%                          % plot the new stuff separately
%                          new_meteo = squeeze(xx_meteo_new(k,p,:));
%                          new_date  = xx_date_new';
%                          plot( new_date, new_meteo, 'b' );
%                      
%                          xlim( [ min(tmp_date) max(tmp_date) ] );
%                          datetick( 'x', 'keeplimits', 'keepticks' );
%                          title( strrep( plist(p).short, '_', '\_' ) );
%                          hold off;
%                      end
%                                           
%                      nn_meteo  = length(tmp_meteo);
%                      tmp_meteo = reshape(tmp_meteo,length(xx_hours),nn_meteo/length(xx_hours))';
%                      tmp_days  = floor(reshape(tmp_date,length(xx_hours),nn_meteo/length(xx_hours)))';
%         
%                      % just a safety check...
%                      if any( any( bsxfun( @minus, tmp_days, tmp_days(:,1) ) ) )
%                          error( 'opaq_import_gfs : date/time array mismatch, fatal error, contact bino...' );
%                      end
%         
%                      xx_meteo = [ xx_meteo; tmp_days(:,1) p*ones(length(tmp_days),1) tmp_meteo ];
%                                                                
%                  end % end loop over variables list        
%                  
%                  
%                  if opts.plot    
%                      % save plot
%                      plfile = fullfile( opts.output, sprintf( 'opaq_gfs_import_%.0f_%.0f-%s.png', ...
%                          10*coords(k,1), 10*coords(k,2), datestr( basetime_utc, 'yyyymmdd.HH' ) ) );
%                      fprintf( 'Saving %s...\n', plfile );
%                      saveas( hFig, plfile, 'png' );
%                      close( hFig );
%                  end
%                  
%                  % save ovl db
%                  save( dbname, 'xx_meteo', 'xx_hours', 'tzone', 'gfs_base' );
                 
            end % switch : update or create new
            
        end % -- loop over the coordinates
        
        
    % -- updating....    
    case 'XXXXX'
        
        % IMPLEMENT ME : SOMETHING ELSE TO UPDATE.... ASCII FILES PROBABLY
        %                SEE OPAQ meteo importer...
                
        
    otherwise
        error( 'This mode is not implemented yet...' );
end


% -- Say goodbye...
fprintf( 'All done, have a nice day :)\n' );


%% print_usage
%  prints a usage message
function print_usage
fprintf( 'Usage:\n');
fprintf( '   opaq_import_gfs [options] LON1,LAT1;LON2,LAT2 YYYYMMDD HH\n');
fprintf( '\n');
fprintf( ' Available options:\n');
fprintf( '   --help ................ : this message\n');
fprintf( '   --repo <path> ......... : path to repository, excl year (def. gfs)\n' )
fprintf( '   --domain <name> ....... : name for the domain (def. china)\n');
fprintf( '   --tzone <shift> ....... : conversion from UTC to local time zone\n');
fprintf( '                             default 0 : no conversion. for conversion to\n');
fprintf( '                             CST use --tzone 8 (i.e. add 8 hours to UTC)\n');
fprintf( '   --mode <name> ......... : select output/import mode : \n');
fprintf( '                              - ovldb  : make ovl matlab db files (def.)\n');
fprintf( '                              - ovlasc : write ovl ascii files\n');
fprintf( '   --output <path> ....... : output folder\n');
fprintf( '   --plots ............... : make a plot in output folder of the update\n' );
fprintf( '\n');
fprintf( ' Bino Maiheu, (c) VITO 2014 \n');
fprintf( ' Contact: bino.maiheu@vito.be\n');
