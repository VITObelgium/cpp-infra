%RIO_DETREND
% This routine removes the spatial bias from the data... performs the
% detrending step geven the current configuration and parameters loaded
% in the config file...
%  
% xx_detr = rio_detrend( cnf, st_info, xx_data )
%
% Note that st_info here is the subset from the st_info in the cnf
% structure, and should be retrieved from rio_dblookup. This routine needs
% the statistics parameters loaded in the configuration structure under
% cnf.xx_avg
%
%
% Detrended concentrations below 0 are put to 0, except when using
% the log transform in RIO
%
% See also rio_init, rio_addtrend, rio_avgtrend, rio_stdtrend,
%          rio_dblookup
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function xx_detr = rio_detrend( cnf, st_info, xx_data )

% Sanity check...
if ~isempty( setdiff(xx_data(:,1), st_info(:,1) ) ) || ...
    ~isempty( setdiff(st_info(:,1), xx_data(:,1) ) )
    error( 'rio_detrend:: data and st_info does not match!!!!' );
end

% Extract the station indicator from the st_info subset with good stations
if ~strcmp( cnf.gis_type, 'CorineID_double_beta' )
    st_indic = st_info(:,4);
else
    st_indic = st_info(:,4:5);
end

%-- Rescale data accoring to trend in standard deviation...
sc_data = detrend_shape( cnf, xx_data, st_indic );

%-- Get the trend for the station indicators...
[ fit, ref_level ] = rio_avgtrend( cnf, st_indic );  

%-- Calculate trend shift...
delta_tr = ref_level - fit;

%-- Detrend pollutant data
xx_detr = sc_data(:,2) + delta_tr;

%-- Add station info...
xx_detr = [ xx_data(:,1) xx_detr];

if ~cnf.Option.logtrans
  %-- Put negative values in xx_detrend to zero!
  neg_i = find(xx_detr(:,2) < 0);
  xx_detr(neg_i,2) = 0;
end

end

% private function...
function scaled_data = detrend_shape( cnf, data, st_indic )

%----------------------------------------
%-- Rescale data set according to trend 
%-- in standard deviation
%----------------------------------------

%-- Get list of stations...
st_list = unique( data(:,1) );
    
%-- Store station mean and stddev in TS that matches 
%-- data structure...
data_avg = zeros( size( st_list ) );

for i=1:length(st_list)
    st_nr = st_list(i);
    
    %-- Find station data
    st_i      = find( data(:,1) == st_nr );
    st_stat_i = find( cnf.xx_avg(:,1) == st_nr );
    
    data_avg( st_i, 1 ) = cnf.xx_avg(st_stat_i, 2 );      
end

% The std_ref returns the standard deviation reference
% needed this with the tests for the uncertainty calculation, not anymore
[ std_scale_fac, std_ref ] = rio_stdtrend( cnf, st_indic );

scaled_data = ( data(:,2) - data_avg ) .* std_scale_fac + data_avg;
scaled_data = [data(:,1) scaled_data];
end

