%RIO_EXCEEDANCES
% Calculates the exceedances, the uncertainty is calculated from the
% uncertainty on the yearly average, see Denby et al....
%  
%  [ noe_grid, noe_st ] = rio_exceedances( cnf, dates, avg_err, method, limit, max_exceed )
%  [ noe_grid, noe_st ] = rio_exceedances( cnf, dates, avg_err, method, limit, max_exceed, path )
%
% The code is a bit of a copy-paste of rio_average, but for this one we
% have to store the entire dataset if we want to use the Monte Carlo
% method
%
% Normally we only calculate the exceedances for the aggregation time
% provided by the configuration cnf.agg_time, however, for pm10 and so2
% we want to compute the exceedances of the 24h average starting from the
% 1h values as well...
%  
% 1h exceedance limits
% -> NO2 : 200 µg/m3, not more than 18 times
% -> SO2 : 350 µg/m3, not more than 24 times
% 24h exceedance limits
% -> PM10 : 50 µg/m3, not more than 35 days
% -> SO2  : 125 µg/m3, not more than 3 times
%
% Output structure
% noe_grid : 
% <id> <noe> <noe_low> <noe_upp> <noe_exceed_prob> [ <da_noe> <da_noe_low> <da_noe_upp> <da_noe_exceed_prob>]
%
% noe_st :
% <id> <noe> [ <da_noe> ]
%
% Note that the da_noe fields are only present when the aggregation time
% is 1h and the pollutants have an exceedance limit on the daily averages
% This is the case for pm10 and so2
%
% Changelog
%  - 2010.08.04 (BM) : added optional argument with path
%
% See also rio_init, rio_average, rio_exceedlimits, rio_outputfilenames, 
%          rio_exceedprobmc
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ noe_grid, noe_st ] = rio_exceedances( cnf, dates, ...
    avg_err, method, limit, max_exceed, varargin )

if ~cnf.have_grid
    error( 'rio_exceedances:: need to have the grid definition loaded in config' );
end
if ~cnf.have_stations
    error( 'rio_exceedances:: need to have stations loaded in config' );
end

outpath = cnf.outpath;
if nargin > 6
   outpath = varargin{1};
end

%-- Allocate the noe grid
noe_grid      = zeros( cnf.grid_n, 9 );
noe_grid(:,1) = cnf.grid_info(:,1);
noe_st        = zeros( cnf.nr_st, 3 );
noe_st(:,1)   = cnf.st_info(:,1);

if cnf.agg_time == 4 
    agg_time_loc = [1:24] + 1; % yes, you DO need the brackets !
else
    agg_time_loc = 2;    
end

% Do we have to calculate the day average from the 1h values ?
if length( limit ) > 1
    calc_dayavg = true;    
else
    calc_dayavg = false;    
end

if strcmp( method, 'montecarlo' )
    val_all = [];
    err_all = [];
end

fprintf( 'rio_exceedances:: exceedance limits & max exceedances :\n' );
disp( limit ), disp( max_exceed );

% -- Now loop over the data again to calculate the exceedances
%    if we want to use the monte carlo method, we have to store everything
%    into a big array (take care of memory issues here for the larger grids
%    and 1 hrs aggregation time)
fprintf( 'rio_exceedances:: looping over outputfiles (this can take a while)...\n' );
for date = dates
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
    
    % Calculate the 24h average for pm10 and so2 from the interpolated 1h
    % grids...
    if calc_dayavg
        val_da = nanmean( val(:,agg_time_loc), 2 );
        st_da  = nanmean( st(:,agg_time_loc-1), 2 );
        
        %-- NOT SURE WHAT TO DO HERE, A BIT EXPERIMENTAL...
        %if strcmp( method, 'montecarlo' )
        %    % Calculate the covariance correction factor for the grid on the
        %    % 24 1h values for this day, only need this for the monte carlo
        %    % method
        %    fcov_24h = rio_calcfcov( val(:,agg_time_loc) );
        %    fprintf( 'rio_exceedances:: fcov for %s 24h averaging = %f\n', datestr( date ), fcov_24h );
        %    err_da   = sqrt( fcov * mean( err(:,agg_time_loc), 2 ).^2 );
        %end
    end
        
    % aggregeate
    switch( method )
        case 'denby'
            %-------------------------------------------------------------
            % don't have to store the big arrays, calculate the exceedances
            % from the grid
            
            
            % Loop over the grid...
            for l=1:size( val, 1 )
                for k=1:length( agg_time_loc ) % loop in case we have 24 values...
                    % simple number of exceedances above the limit for the
                    % aggregation time
                    if limit(1) > 0
                        if val(l,agg_time_loc(k)) > limit(1)
                            noe_grid(l,2) = noe_grid(l,2) + 1;
                        end
                        % noe_lower : method according to denby, value - error on year avg
                        % note that here we also apply the uncertainy on the yearly
                        % average on a 1h basis
                        if ( val(l,agg_time_loc(k))- avg_err(l) ) > limit(1)
                            noe_grid(l,3) = noe_grid(l,3) + 1;
                        end
                        % noe_upper
                        if ( val(l,agg_time_loc(k))+ avg_err(l) ) > limit(1)
                            noe_grid(l,4) = noe_grid(l,4) + 1;
                        end
                    else
                        noe_grid(l,2) = -1;
                        noe_grid(l,3) = -1;
                        noe_grid(l,4) = -1;
                    end
                    
                end
                
                % if we have a day average calculated, compute the noe for this...
                if calc_dayavg && ( limit(2) > 0 )
                    if val_da(l) > limit(2)
                        noe_grid(l,6) = noe_grid(l,6) + 1;
                    end
                    if ( val_da(l) - avg_err(l) ) > limit(2)
                        noe_grid(l,7) = noe_grid(l,7) + 1;
                    end
                    if ( val_da(l) + avg_err(l) ) > limit(2)
                        noe_grid(l,8) = noe_grid(l,8) + 1;
                    end
                else
                    noe_grid(l,6) = -1;
                    noe_grid(l,7) = -1;
                    noe_grid(l,8) = -1;
                end
            end
            
            % Loop over the stations...
            for s = 1:size(st,1)
                for k = 1:length(agg_time_loc )
                    if limit(1) > 0
                        if st(s,agg_time_loc(k)-1 ) > limit(1)
                            noe_st(s,2) = noe_st(s,2) + 1;
                        end
                    else
                        noe_st(s,2) = -1;
                    end
                end
                
                % If we have a day average calculated, compute the noe as well...
                if calc_dayavg && ( limit(2) > 0. )                    
                    if st_da(s) > limit(2)
                        noe_st(s,3) = noe_st(s,3) + 1;
                    end
                else
                    noe_st(s,3) = -1;
                end
                
            end
            
        case 'montecarlo'            
            %-------------------------------------------------------------
            %-- just store the big arrays
            if cnf.agg_time == 3
                % only implement this for da
                val_all = [ val_all val(:,agg_time_loc)  ];
                err_all = [ err_all err(:,agg_time_loc)  ];                
            else
                error( 'rio_exceedances:: Monte Carlo method only implemented for da agg_time\n' );
            end
            
        otherwise
            error( 'rio_exceedances:: unknown method %s\n', method );
    end
    
end


% Now process the info, and compute the exceedance probability from the
% confidence interval on NOE or runs some monte carlo to get it...

switch( method )
    case 'denby'
        % -----------------------------------------------------------------
        % We have the confidence intervals on the NOE
        
        % Here we would need the cumulative distribution of number of 
        % exceedances to compute a turstworthy value for the probability of
        % exceedance. Don't know if gevpdf distributions are applicatble
        % here ??, so for now... just use gaussian distribution...
        
        % We take the average here as sigma...
        sig = .5 * ( noe_grid(:,4) - noe_grid(:,3) );        
        noe_grid(:,5) = 100. * ( 1. - rio_normcdf( max_exceed(1), noe_grid(:,2), sig ) );
        noe_grid( isnan( noe_grid(:,5) ),5 ) = 0.;
        
        if calc_dayavg        
            sig = .5 * ( noe_grid(:,8) - noe_grid(:,7) );
            noe_grid(:,9) = 100. * ( 1. - rio_normcdf( max_exceed(2), noe_grid(:,6), sig ) );
            noe_grid( isnan( noe_grid(:,9) ),9 ) = 0.;
        end
        
    case 'montecarlo'
        % -----------------------------------------------------------------
        % Run a MC simulation to get the exceedance probability
        % we have : val_all, err_all
        
        % Calculate the temporal correlation matrix for the entire grid
        rho = corrcoef( val_all );
        
        % Loop over every grid cell...
        for i=1:size( val_all, 1 )
            val_ts = val_all(i,:);
            err_ts = err_all(i,:);
           
            fprintf( 'rio_exceedances:: mc generation for pixel %d/%d\n', i, size( val_all, 1 ) );
            noe_grid(i,5) = rio_exceedprobmc( rho, val_ts, err_ts, limit(1), max_exceed(1) );
            noe_grid(i,9) = -1;
        end       
    
    
    otherwise
        error( 'rio_exceedances:: unknown method %s\n', method );
end


end