%RIO_DBLOOKUP
% Looks up the requested data from the historic database and returns the
% the station data ( with good data, so not NaN (or -9999) ) and the 
% corresponding station info list.
% 
% [st_info, xx_data] = rio_dblookup( cnf, date )
%
% Returned is a structure with the station data and the measurement values.
% Depending on the configuration in cnf; different daily aggregation times
% are returned. The data is searched in the cnf.xx_val and cnf.xx_date
% arrays in the rio configuration structure
%
%
% As of v1.4 I've added the check on cnf.stationList -> read from the XML
% file, if a station is not present in the XML list, we do not use it...
%
% See also rio_init rio_setup
% 
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [st_info, xx_data] = rio_dblookup( cnf, date )

%---------------------------------------
%-- Trim station and xx_data vectors on
%-- available information
%---------------------------------------
% a small check...
if ~cnf.have_db
   error( 'rio_dblookup:: The historic database is not set up yet, run rio_loaddb() first !' ); 
end

% find the column index for the requested data
if cnf.agg_time <= 3
    agg_time_loc = cnf.agg_time + 1;
elseif cnf.agg_time == 4
    %-- Trunc date...
    vec_date = datevec(date);
    date = datenum(vec_date(1:3));
    hour = vec_date(4);
    agg_time_loc = hour + 1 + 1;
    %-- +1 for shift of station nr, +1 for conversion of hour to col nr.
    %-- It is assumed that first row of 1h val corresponds to Oh of
    %-- that day!
end

%-- Select xx data for selected date
date_i =  find( cnf.xx_date == date );

%-- Sort data on station number
tmp_xx_data = sortrows( cnf.xx_val( date_i,: ) );

%-- Store info in data structure 
%-- (1e col = station number ; 2e col = XX pollutant value)
xx_data(:,1) = tmp_xx_data(:,1);
xx_data(:,2) = tmp_xx_data(:,agg_time_loc);

%-- Dump all Nan values that might exist in the 1hval...
i_nan = find(isnan(xx_data(:,2)) == 1);
xx_data(i_nan,:) = [];

%-- if we have defined some station list, make sure we use only this
% list...
if isfield( cnf, 'stationList' ) && ~isempty( cnf.stationList )
    %fprintf( '+++ WARNING: applying xml station list in rio_dblookup, still experimental, a bit inefficient as well...\n' );
    [ tmp, i_sel, i_list ] = intersect( {cnf.st_id{xx_data(:,1)}}, cnf.stationList );
    xx_data = xx_data(i_sel,:);
end


%-- Attention: xx_data contains available station info, 
%-- based on input data. Make sure that this is matching the 
%-- selection of stations in stat_info...

[stat_intersect, i_xx_data, i_st_info] = intersect(xx_data(:,1), cnf.st_info(:,1));

%-- Trim both data bases...
st_info = cnf.st_info(i_st_info,:);
% st_id = st_id(i_st_info);
xx_data = xx_data(i_xx_data,:);

if isempty(xx_data)
    fprintf( '++ no measurements are availble for %s !!\n', datestr(date) );
    return;
end

end
