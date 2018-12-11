%RIO_LOADSTATIONINFO
% This routine loads the station info and return a list of stationID's and
% corresponding info
%  
%  [ cnf] = rio_loadstationinfo( cnf )
%  [ cnf] = rio_loadstationinfo( cnf, station_file )
%
% See also rio_init, rio_loadgrid, rio_loaddb
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ cnf ] = rio_loadstationinfo( cnf, varargin )

cnf.errcode   = 0;
cnf.errmsg    = '';

%---------------------
%-- Store station info 
%---------------------

%-- Data file
if nargin > 1
    station_info_file = varargin{1};
else
    station_info_file = fullfile( cnf.stationsPath, ...
    sprintf( '%s_stations_info_GIS_%s.txt', cnf.pol_xx, cnf.gis_type ) );
end

if ~exist( station_info_file, 'file' )
    cnf.errcode = 1;
    cnf.errmsg  = sprintf( 'no such file : %s', station_info_file );
    return;
else
   fprintf( 'Loading station information from %s\n', station_info_file ); 
end

% -- BM:2016-12 : changing this importer to a more modern format
db = importdata( station_info_file );

cnf.st_id         = db.textdata(2:end,2);

% depending on the number of columns : we have the altitude in the input files or not... 
if size(db.data,2) == 5
    cnf.st_info       = [ str2double( db.textdata(2:end,1) ) db.data(:,1:2) db.data(:,5) db.data(:,4) ];
    cnf.st_indic      = db.data(:,5);
elseif size(db.data,2) == 4
    warning( '+++ Reading old station format without the altitude field...' );
    cnf.st_info       = [ str2double( db.textdata(2:end,1) ) db.data(:,1:2) db.data(:,4) db.data(:,3) ];
    cnf.st_indic      = db.data(:,4);
end

cnf.nr_st         = size( cnf.st_info, 1 );
cnf.have_stations = true;

end