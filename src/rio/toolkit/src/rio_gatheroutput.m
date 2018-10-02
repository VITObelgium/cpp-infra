%RIO_GATHEROUTPUT
% The optional argument grid_idx allows the user to retrieve a subset of
% the grid, the export (boolean) tells the routine whether to dump a
% mat-file with the gathered data?
%
% xx_data = rio_gatheroutput( cnf, dates, path )
% xx_data = rio_gatheroutput( cnf, dates, path, grid_idx )
% xx_data = rio_gatheroutput( cnf, dates, path, grid_idx, export )
%
% Changelog
%  - 2010.08.04 (BM) : explicitly added the output path here !
%
% See also rio_init, rio_outputfilenames, rio_average, rio_exceedances
% 
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu
 
function xx_data = rio_gatheroutput( cnf, xx_dates, path, varargin )

export = false;

if ~cnf.have_grid
    error( 'rio_gatheroutput:: need to have the grid definition loaded in config' );
end

if nargin > 3
    % array of grid indices....
    grid_idx = varargin{1};
    if nargin > 4
        export = varargin{2};
    end
else    
    % all of the grid cells
    grid_idx = 1:cnf.grid_n;
end


if cnf.agg_time <= 3
    agg_time_loc = 2;
elseif cnf.agg_time == 4
    agg_time_loc = [1:24] + 1; % yes, you DO need the brackets !!
end

% Init the data structure
xx_data = struct( 'grid', [], 'err', [], 'obs', [] );
fprintf( 'rio_gatheroutput:: TODO: ADJUST THIS ROUTINE (+outputfilenames)TO NEW XML BASE CONFIG....\n' );
fprintf( 'rio_gatheroutput:: gathering outputfiles, this can take a while...\n' );
for date = xx_dates
    % Get what the output filenames should be for the current date...
    [ grid_file, err_file, st_file ] = rio_outputfilenames( cnf, date, path );

    % Do they exist ?
    if ~exist( grid_file, 'file' ) ...
            || ~exist( err_file, 'file' ) ...
            || ~exist( st_file, 'file' )
       error( 'rio_gatheroutput:: %s output not found, please generate first...', datestr( date ) ); 
    end
    
    % read the data and the errors
    tmp  = importdata( grid_file );
    tmp2 = importdata( err_file );
    tmp3 = importdata( st_file );
    
    % select subset for grid...
    val = tmp.data( grid_idx, : );
    err = tmp2.data( grid_idx, : );
    st  = tmp3.data;
    
    if cnf.outputXY
       val(:,2:3)   = [];
       err(:,2:3)   = [];
       st(:,1:2)    = [];
    end
    
    % Put NaN instead of -999 in station data... 
    st( st == cnf.missing_value ) = NaN;
        
    xx_data.grid = [ xx_data.grid val(:,agg_time_loc) ];
    xx_data.err  = [ xx_data.err  err(:,agg_time_loc) ];
    xx_data.obs  = [ xx_data.obs   st(:,agg_time_loc-1)  ];    
end

% Export a file with the RIO data...
if export    
    fname = sprintf( '%s_data_%s_%s-%s-%s-%s.mat', cnf.pol, ...
        cnf.ipol_mode, cnf.grid_type, cnf.at_lb, ...
        datestr( xx_dates(1), 'yyyy.mm.dd' ), datestr( xx_dates(end), 'yyyy.mm.dd') );
    fprintf( 'rio_gatheroutput:: exporting gathered data to %s\n', fname );
    save( fname, 'xx_data' );
end

end