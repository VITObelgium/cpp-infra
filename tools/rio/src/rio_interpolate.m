%RIO_INTERPOLATE
% Main RIO interpolation routine, depending on the configuration structure,
% the full RIO interpolation model is run, or we use ordinary kriging, or 
% IDW(i).
%
%  [ xx_data, xx_grid ] = rio_interpolate( cnf, date )
%  [ xx_data, xx_grid ] = rio_interpolate( cnf, values ) (not implemented)
%
% Input arguments are obviously the configuration structure, followed by
% either a date in matlab format or a list with values corresponding to
% the number of stations. So the user can either give a date, in which
% case the routine will see if a historic database is loaded and lookup
% the station measurements, or give an array with measurement values; 
% which should be of the same size as the station list, in which case
% the routine will directly interpolate from those values.
%  
% The routine always will return the measurement values and the
% interpolation grid, if no good measurements are found for that day, the
% routine will return a station file and grid with missing values filled
% in.
%
% History
%  - BM. 12-04-2010 : created
%  - BM. 16-04-2010 : ported the IDW and IDWi to the new library, still have to
%    test this
%
% See also rio_init, rio_dblookup, rio_detrend, rio_covmat, rio_krigegrid,
%          rio_addtrendgrid, rio_idwgrid, rio_idwigrid
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ xx_data, xx_grid ] = rio_interpolate( cnf, arg )

if ~cnf.have_stations, error( 'rio_interpolate:: no stations loaded...' ); end;
if ~cnf.have_spcorr,   error( 'rio_interpolate:: no spatial correlations loaded...' ); end;
if strcmp( cnf.ipol_mode, 'RIO' )
    if ~cnf.have_trend,    error( 'rio_interpolate:: no trend loaded...'); end;
end
if ~cnf.have_stats,    error( 'rio_interpolate:: no long term stats loaded...' ); end;


%% First setup & trim the data to prepare interpolation
if numel( arg ) == cnf.nr_st
    error( 'Direct interpolation from data not implemented yet...' );    
    % here we have to build the st_info_tmp and xx_vals from the supplied
    % data...
    
    % the user will have to make sure that the correct trend functions are
    % loaded in the config structure as we don't check then for the
    % date to select week/weekend...
    
else
    if ~cnf.have_db, error( 'rio_interpolate:: no historic data loaded...' ); end;
    
    % Assume the user supplied a datenum to interpolate from, in which case
    % we look up the data in the database & trim it, the routine rio_dblookup
    % used to be the old trim_data routine... 
    date = arg;
    [st_info_tmp, xx_data] = rio_dblookup( cnf, date );
    
end
  
% Do we have good data for this date/time ?
if isempty(xx_data) 
   fprintf( '++ warning:: no good data for %s\n', datestr( date ) );
   xx_data = [ cnf.st_info(:,1)  cnf.missing_value*ones(cnf.nr_st, 1) ];
   xx_grid = [ cnf.grid_info(:,1) cnf.missing_value*ones(cnf.grid_n, 1) zeros(cnf.grid_n,1) ];      
   return; 
end

%% Interpolate to the specific mode requested in the cnf structure
switch cnf.ipol_mode
    case 'RIO'
        %--------------------------------
        %-- RIO interpolation scheme...
        %--------------------------------    
        
        %-- Detrend the data values 
        xx_detr = rio_detrend( cnf, st_info_tmp, xx_data );      
        
        %-- Get inverse correlation matrix, data values are not needed here
        %   since for now we have kicked out the calculation of the actuall
        %   spatial correlations on the data (see tests RIO 2008)
        C_inv   = rio_covmat( cnf, st_info_tmp );
                        
        %-- Interpolate grid using ordinary kriging
        xx_grid = rio_krigegrid( cnf, C_inv, st_info_tmp, xx_detr );
        
        %-- Add the trend again...
        xx_grid = rio_addtrendgrid( cnf, xx_grid );
    
    case 'OrdKrig'        
        %-------------------------------------------------------
        %-- Ordinary Kriging interpolation (no detrending)...
        %-------------------------------------------------------  
        
        %-- Get inverse correlation matrix
        C_inv   = rio_covmat( cnf, st_info_tmp );
                        
        %-- Interpolate grid using ordinary kriging
        xx_grid = rio_krigegrid( cnf, C_inv, st_info_tmp, xx_data );
        
        
    case 'IDW'
        %----------------------------------
        %-- IDW interpolation scheme...
        %----------------------------------
        xx_grid = rio_idwgrid( cnf, st_info_tmp, xx_data );
        
        
    case 'IDWi'
        %--------------------------------------------------------------
        %-- IDW interpolation scheme according to IRCEL modifications...
        %--------------------------------------------------------------
        xx_grid = rio_idwigrid( cnf, st_info_tmp, xx_data);        
        
        
    otherwise
        error( 'rio_interpolate:: unknown mode %s', cnf.ipol_mode );
        return;
end

% Rebuild station data for export, set RIO missing value
curr_stat = cnf.missing_value * ones( cnf.nr_st, 1 );
for i = 1:cnf.nr_st
    [tf, loc] = ismember( i, xx_data(:,1) );
    if tf, curr_stat( i ) = xx_data(loc,2);  end;
end
xx_data = [ cnf.st_info( :,1) curr_stat ]; 
        

end
