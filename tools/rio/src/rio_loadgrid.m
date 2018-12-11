%RIO_LOADGRID
% This routine attaches the grid information to the RIO config
% structure...
%  
% [ cnf ] = rio_loadgrid( cnf )
%
% See also rio_init, rio_loadstationinfo, rio_loaddb
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ Cnf ] = rio_loadgrid( Cnf )

Cnf.errcode   = 0;
Cnf.errmsg    = '';

%-------------------------
% -- Load the grid for the specific pollutant, GIS and grid type
% -- complains if the combination is not available...
%-------------------------

%-- initialize
Cnf.grid_info = [];

%-- do we have a grid type configured ?
if isempty( Cnf.grid_type ) 
  error( 'RIO:LOADGRID', 'No grid type defined in the config structure...\n' );
end


%-- Set the filename
grid_info_file = fullfile( Cnf.driversPath, ...
    sprintf( '%s_grid_%s.txt', Cnf.gis_type, Cnf.grid_type ) );

%-- Does it exist ?
if ~exist( grid_info_file, 'file' ),
    Cnf.errcode = 1;
    Cnf.errmsg  = sprintf( 'file not found %s', grid_info_file );
    return;
else
    fprintf( 'Loading interpolation grid from %s\n', grid_info_file );
end

%-- Open file
fid = fopen(grid_info_file);

%-- Dump header...
fgetl(fid);

%-- Read a line and interpret content
if ~strcmp( Cnf.gis_type, 'CorineID_double_beta' )
    C =  textscan(fid, '%f %f %f %f');
    Cnf.grid_info = [C{1,1} C{1,2} C{1,3} C{1,4}];
else
    C =  textscan(fid, '%f %f %f %f %f');
    Cnf.grid_info = [C{1,1} C{1,2} C{1,3} C{1,4} C{1,5}];  
end 

Cnf.grid_n    = size( Cnf.grid_info, 1 );
switch ( Cnf.grid_type )
    case 'belEUROS'
        Cnf.grid_res  = 3000;
    case '4x4'
        Cnf.grid_res  = 4000;
    case 'cn2018'
        Cnf.grid_res  = 4000;
    case '5x5'
        Cnf.grid_res  = 5000;
    case '3x3'
        Cnf.grid_res  = 3000;
    case '1x1'
        Cnf.grid_res  = 1000;
    otherwise
        Cnf.errcode = 2;
        Cnf.errmsg  = sprintf( 'cannot set grid resolution for type %s', Cnf.grid_type );
        return;
end

Cnf.have_grid = true;

fclose(fid);
end
