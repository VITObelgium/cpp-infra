%RIO_H5CREATE
%
% Create a RIO hdf5 output file for storing interpolation results
% 
%  fh = rio_h5create( cnf, fname )
%
% The routine returns a filehandle structure with hdf5 info
%
% See also rio_init, rio_h5close, rio_h5append
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function fh = rio_h5create( cnf, fname )

fh       = struct();
fh.fname = fname;
fh.h5fID = H5F.create( fname, 'H5F_ACC_TRUNC', 'H5P_DEFAULT', 'H5P_DEFAULT' );

% create groups
fh.h5GridGroupID = H5G.create( fh.h5fID, 'grid', 512 );
fh.h5StatGroupID = H5G.create( fh.h5fID, 'stations', 512 );
fh.h5TimeGroupID = H5G.create( fh.h5fID, 'time', 512 );


% Set some values based upon the configuration
rio_h5attr( fh.h5fID, 'pollutant', cnf.pol_xx );
rio_h5attr( fh.h5fID, 'agg_time', cnf.at_lb );
rio_h5attr( fh.h5fID, 'gis_type', cnf.gis_type );
rio_h5attr( fh.h5fID, 'grid_type', cnf.grid_type );
rio_h5attr( fh.h5fID, 'ipol_mode', cnf.ipol_mode );

rio_h5attr( fh.h5fID, 'code_implementation', 'matlab' );
rio_h5attr( fh.h5fID, 'code_version', sprintf( 'library version %d.%d', cnf.version.major, cnf.version.minor ) );
rio_h5attr( fh.h5fID, 'config_version', cnf.configName );
rio_h5attr( fh.h5fID, 'kriging_mode', 'n/a' );
rio_h5attr( fh.h5fID, 'output_format', '1.1' );


h5writeatt( fname, '/', 'missing_value', int32(-9999) );
h5writeatt( fname, '/', 'detection_limit', cnf.detection_limit );
h5writeatt( fname, '/', 'scale_factor', cnf.scale_factor );

% Set some grid values based upon the configuration
rio_h5write( fh.h5GridGroupID, 'x', single( cnf.grid_info(:,2) ) );
rio_h5write( fh.h5GridGroupID, 'y', single( cnf.grid_info(:,3) ) );

rio_h5attr( fh.h5GridGroupID, 'grid_space', single( cnf.grid_res ) );
if ~strcmp( cnf.gis_type, 'CorineID_double_beta' )
    rio_h5write( fh.h5GridGroupID, 'driver', single( cnf.grid_info(:,4) ) );
else
  fprintf( '+++ Not writing grid to hdf5 for double beta...\n' );
end
rio_h5attr( fh.h5fID, 'run_start', datestr( now, 31 ) );

% Set some station values
rio_h5write( fh.h5StatGroupID, 'x', single( cnf.st_info(:,2) ) );
rio_h5write( fh.h5StatGroupID, 'y', single( cnf.st_info(:,3) ) );
rio_h5write( fh.h5StatGroupID, 'driver', single( cnf.st_info(:,4) ) );
rio_h5write( fh.h5StatGroupID, 'name', cell2mat(cnf.st_id) );

% output index
fh.curr_idx = 0;

% create the concentration/uncertainty variables
fh.grid_dims    = [ cnf.grid_n 1 ];
fh.grid_maxdims = {  cnf.grid_n 'H5S_UNLIMITED' };
fh.grid_space   = H5S.create_simple( 2, fh.grid_dims , fh.grid_maxdims );
paramID         = H5P.create( 'H5P_DATASET_CREATE' );
H5P.set_chunk( paramID, fh.grid_dims );
fh.concID = H5D.create( fh.h5GridGroupID, 'value', ...
    'H5T_NATIVE_FLOAT', fh.grid_space, paramID );
fh.errorID = H5D.create( fh.h5GridGroupID, 'error', ...
    'H5T_NATIVE_FLOAT', fh.grid_space, paramID );
fh.grid_memspaceID = H5S.create_simple( 1, cnf.grid_n, [] );

h5writeatt( fname, '/grid/value', 'units', 'ug/m3' );
h5writeatt( fname, '/grid/value', 'description', 'gridded concentrations' );
h5writeatt( fname, '/grid/value', 'missing_value', int32(-9999) );
h5writeatt( fname, '/grid/value', 'scale_factor', cnf.scale_factor );

h5writeatt( fname, '/grid/error', 'units', 'ug/m3' );
h5writeatt( fname, '/grid/error', 'description', 'error on gridded concentrations' );
h5writeatt( fname, '/grid/error', 'missing_value', int32(-9999) );
h5writeatt( fname, '/grid/error', 'scale_factor', cnf.scale_factor );


% create the station variables
fh.stat_dims    = [ cnf.nr_st 1 ];
fh.stat_maxdims = { cnf.nr_st 'H5S_UNLIMITED' };
fh.stat_space   = H5S.create_simple( 2, fh.stat_dims , fh.stat_maxdims );
paramID         = H5P.create( 'H5P_DATASET_CREATE' );
H5P.set_chunk( paramID, fh.stat_dims );
fh.st_concID = H5D.create( fh.h5StatGroupID, 'value', ...
    'H5T_NATIVE_FLOAT', fh.stat_space, paramID );
fh.stat_memspaceID = H5S.create_simple( 1, cnf.nr_st, [] );

h5writeatt( fname, '/stations/value', 'units', 'ug/m3' );
h5writeatt( fname, '/stations/value', 'description', 'measured concentrations' );
h5writeatt( fname, '/stations/value', 'missing_value', int32(-9999) );
h5writeatt( fname, '/stations/value', 'scale_factor', cnf.scale_factor );

% create the time variables
fh.time_dims    = [ 1 ];
fh.time_maxdims = { 'H5S_UNLIMITED' };
fh.time_space   = H5S.create_simple( 1, fh.time_dims , fh.time_maxdims );
paramID         = H5P.create( 'H5P_DATASET_CREATE' );
H5P.set_chunk( paramID, fh.time_dims );
fh.yearID = H5D.create( fh.h5TimeGroupID, 'year', ...
    'H5T_NATIVE_INT16', fh.time_space, paramID );
fh.monthID = H5D.create( fh.h5TimeGroupID, 'month', ...
    'H5T_NATIVE_INT16', fh.time_space, paramID );
fh.dayID = H5D.create( fh.h5TimeGroupID, 'day', ...
    'H5T_NATIVE_INT16', fh.time_space, paramID );
if cnf.agg_time == 4
    fh.hourID = H5D.create( fh.h5TimeGroupID, 'hour', ...
        'H5T_NATIVE_INT16', fh.time_space, paramID );
end

fh.time_memspaceID = H5S.create_simple( 1, 1, [] );

