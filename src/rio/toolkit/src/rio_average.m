%RIO_AVERAGE
%  This routine calculates the average of an interpolated dataset, it also
%  calculates the exceedance probability of the norm on yearly average
%  concentrations...
%  
%  [ avg_grid, avg_st, fcov ] = rio_average( cnf, dates )
%  [ avg_grid, avg_st, fcov ] = rio_average( cnf, dates, path )
%
% Changelog
%  - 2010.08.04 : explicitly added the outputpath...
%
% See also rio_init, rio_gatheroutput, rio_calcfcov, rio_outputfilenames,
%          rio_exceedances
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ avg_grid, avg_st, fcov ] = rio_average( cnf, dates, varargin )

if ~cnf.have_grid
    error( 'rio_average:: need to have the grid definition loaded in config' );
end
if ~cnf.have_stations
    error( 'rio_average:: need to have stations loaded in config' );
end
% -- First calculate the temporal covariance matrix of the dataset, 
%    this must be done upon the grid data. Because some grids are quite
%    large, we select a random subset of grid cells in the interpolation 
%    grid, say 1000 to calculate the covariance matrix for the grid off,
%    this should be sufficient. (was checked, see logbook Bino 20/04/2010)
tmp      = randperm( cnf.grid_n );
grid_idx = tmp( 1:min(1000,cnf.grid_n) );

outpath = cnf.outpath;
if nargin > 2
   outpath = varargin{1};
end

% Maybe foresee a switch to round the values as done in the RIO ircel ?
% data = rio_gatheroutput( cnf, dates, grid_idx, false );
data = rio_gatheroutput( cnf, dates, outpath, 1:cnf.grid_n, false );


if cnf.Option.logtrans,
  warning( 'do not know whether it is best to do the logtrans here again in the postprocessing and when to do the backtransform... what average do we want to compute ???\n' );
  warning( 'not doing it at the moment.... ' );
%     % convert these values back to logarithmic form to handle in the 
%     % post-processing
%     printf( '+++ converting concentrations back to log(1+C)\n' );
%     data.err  = data.err ./ ( 1 + data.grid );    
%     data.grid = log( 1 + data.grid );
%     data.obs  = log( 1 + data.obs );
end

[ fcov ] = rio_calcfcov( data.grid );

fprintf( 'rio_average:: temporal covariance correction : %f\n', fcov );

% -- Booking the output variable
%    the grid : <grid_id> <number> <avg> <avg_err> <stddev> <excee prob>
avg_grid      = zeros( cnf.grid_n, 6 );
avg_grid(:,1) = cnf.grid_info(:,1);
avg_st        = zeros( cnf.nr_st, 3 );
avg_st(:,1)   = cnf.st_info(:,1);

if cnf.agg_time <= 3
    agg_time_loc = 2;
elseif cnf.agg_time == 4
    agg_time_loc = [1:24] + 1; % yes, you DO need the brackets !
end

% -- Now loop over the data again to calculate the average, don't want to
%    load in the all the data in memory, do it in a loop !
fprintf( 'rio_average:: looping over outputfiles (this can take a while)...\n' );
for date=dates
    % Get what the output filenames should be for the current date...
    [ grid_file, err_file, st_file ] = rio_outputfilenames( cnf, date, outpath );
    
    % Do they exist ?
    if ~exist( grid_file, 'file' ) ...
            || ~exist( err_file, 'file' ) ...
            || ~exist( st_file, 'file' )
       error( 'riopp:: %s output not found, please generate first...', ...
           datestr( date ) ); 
    end
    
    % read the data and the errors
    tmp  = importdata( grid_file );
    tmp2 = importdata( err_file );
    tmp3 = importdata( st_file );
    
    % select subset for grid...
    val = tmp.data;
    err = tmp2.data;
    st  = tmp3.data;
    
    % checl sizes
    if size( val, 1 ) ~= cnf.grid_n
        error( 'rio_average:: inconsistent grid size with config\n' );
    end
    if size( st, 1 ) ~= cnf.nr_st
        error(  'rio_average:: inconsistent number of stations with config\n' );
    end
        
    if cnf.outputXY
       val(:,2:3)   = [];
       err(:,2:3)   = [];
       st(:,1:2)    = [];
    end
    
    % Put NaN instead of -999 in station data... 
    st( st == cnf.missing_value ) = NaN;      
    
    % Loop over the grid...
    for l=1:size( val, 1 )
        for k=1:length( agg_time_loc ) % loop in case we have 24 values...
            if val(l,agg_time_loc(k)) > 0
                % number of good data values
                avg_grid(l,2) = avg_grid(l,2) + 1;
                % sum
                avg_grid(l,3) = avg_grid(l,3) + val(l,agg_time_loc(k));                                
                % sum of the uncertainties squared
                avg_grid(l,4) = avg_grid(l,4) + err(l,agg_time_loc(k)).^2;
                % sum of the squares
                avg_grid(l,5) = avg_grid(l,5) + val(l,agg_time_loc(k)).^2;
            end
        end
    end
    
    % Loop over the stations...
    for s = 1:size(st,1)
        for k = 1:length(agg_time_loc )
            if ~isnan( st(s,agg_time_loc(k)-1 ) )
                avg_st(s,2) = avg_st(s,2) + 1;
                avg_st(s,3) = avg_st(s,3) + st(s,agg_time_loc(k)-1);                
            end
        end
    end
        
end

%-- Calculate a bit artisanal to avoid having to use the nanmean etc
% routines which are compromised by the stat toolbox licence...
avg_grid(:,3) = avg_grid(:,3) ./ avg_grid(:,2);
avg_grid(:,4) = sqrt( fcov * avg_grid(:,4)./avg_grid(:,2) );
avg_grid(:,5) = sqrt( avg_grid(:,5) ./ avg_grid(:,2) - avg_grid(:,3).^2 );

% For the stations... (division by 0 gives NaN)
avg_st(:,3) = avg_st(:,3) ./ avg_st(:,2);
avg_st( isnan( avg_st(:,3) ), 3 ) = cnf.missing_value;

% With the uncertainty on the yearly average, compute the exceedance
% probability of the EU norm., Cfr site IRCEL...
switch( cnf.pol )
    case 'pm10'
        eu_norm = 40.;
    case 'so2'
        eu_norm = 20.;
    case 'no2'
        eu_norm = 40.;  
    case 'nh3'
        eu_norm =  8.;  % norm for NH3 yearly average
    otherwise
        eu_norm = -1;    
end

if eu_norm > 0.
   fprintf( 'rio_average:: computing exceedance probability for %s year average > %.1f µg/m3\n', cnf.pol, eu_norm );
   avg_grid(:,6) = 100. * ( 1. - rio_normcdf( eu_norm, avg_grid(:,3), avg_grid(:,4) ) );      
end


end
