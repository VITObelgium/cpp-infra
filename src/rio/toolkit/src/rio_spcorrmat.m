%RIO_SPCORRMAT
% Small helper function to compute the covariance matrix for a set
% of dates xx_date and stations in st_idx. The data are stored in xx_val,
% note that only 1 set of values needs to be present, so the xx_val is 
% [ station_nr value; station_nr value ] for which the xx_date holds the
% corresponding dates. The date from the station numbers is selected
%
% c_mat = rio_spcorrmat( cnf )
% c_mat = rio_spcorrmat( cnf, st_idx )
% c_mat = rio_spcorrmat( st_idx, xx_date, xx_val )
% 
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function c_mat = rio_spcorrmat( varargin )

% -- some parameters we define here, specialist
min_length  = 10;          % minimum length needed in timeseries to get covariance
spcorr_mode = 'cov_corr';  % type of spatial correlations, see code

if nargin < 3
    xx_val  = cnf.xx_val(:, [ 1 cnf.agg_time_loc ] );
    xx_date = cnf.xx_date;
    if nargin == 2
        st_idx  = varargin{1};
    else
        st_idx  = 1:cnf.nr_st;
    end    
elseif nargin == 3
    st_idx  = varargin{1};    
    xx_date = varargin{2};
    xx_val  = varargin{3};   
else
    error( 'rio_spcorrmat: invalid arguments !' );
end

%-- safety checks
if size( xx_date, 1 ) ~= size( xx_val, 1 )
    error( 'rio_spcorrmat: array sizes not equal !' );
end
   
    
%-- pre-allocate...
c_mat = zeros( length(st_idx), length(st_idx) );

%-- here we go...
for i=1:length( st_idx )
    for j=i:length( st_idx )        
        
        % trim the timeseries
        [ xx_1, xx_2 ] =  trim_ts( xx_val, xx_date, st_idx(i), st_idx(j) );
       
        % check the result and compute spatial correlation matrix
        if length( xx_1 ) < min_length
            c_mat(i,j) = NaN;
        else
            switch( spcorr_mode )
                
                case 'cov_corr' % -- covariance
                    c = cov( [ xx_1 xx_2 ] );
                    if ( c(1,1) ~= 0 ) && ( c(2,2) ~= 0 )
                        c_mat(i,j) = c(1,2) / sqrt( c(1,1) * c(2,2) );
                    else
                        c_mat(i,j) = NaN;
                    end
                    
                case 'tmp_corr' % -- Jef H. 2004
                    if i == j
                        c_mat(i,j) = 1.;
                    else
                        len = length(xx_1);
                        c_mat(i,j) = mean((xx_1(2:len) - xx_1(1:len-1)) .* (xx_2(2:len) - xx_2(1:len-1)))/...
                            sqrt((mean((xx_1(2:len) - xx_1(1:len-1)).^2) * mean((xx_2(2:len) - xx_2(1:len-1)).^2)));
                    end
                    
                case 'act_corr' % -- ArcGIS def.
                    c_mat(i,j) = 0.5*sqrt((xx_1 - xx_2)^2);
                
                otherwise
                    error( 'rio_spcorrmat: unknown spatial correlation mode' );                    
            end
        end
        % lower triangle
        c_mat(j,i) = c_mat(i,j);
    end    
end


% -------------------------------------------------------------------------
% libRIO (c) 2010/2011 VITO
% Helper function to trim timeseries
% to use with covariance computation
% -------------------------------------------------------------------------
function [ xx_1 xx_2 ] = trim_ts( xx_val, xx_date, st_1, st_2 )

st_1_i = find( xx_val(:,1) == st_1 );
xx_1   = xx_val( st_1_i, 2 );
date_1 = xx_date( st_1_i );
nan_1  = find( isnan( xx_1 ) );
xx_1(nan_1)   = [];
date_1(nan_1) = []; 

st_2_i = find(xx_val(:,1) == st_2);
xx_2   = xx_val(st_2_i,2);
date_2 = xx_date(st_2_i);
nan_2  = find( isnan(xx_2) );
xx_2(nan_2)   = [];
date_2(nan_2) = [];

mem_1_i = find( ismember( date_1, date_2 ) );
mem_2_i = find( ismember( date_2, date_1 ) );   

xx_1 = xx_1( mem_1_i, : );
xx_2 = xx_2( mem_2_i, : );



