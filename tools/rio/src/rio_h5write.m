%RIO_H5WRITE
%
% Writes some 1D variable to the hdf5 file at the group ID specified. 
% The variable is created in the file. Note that the type is inferred
% from the value matlab type. 
% 
%  rio_h5write( h5id, name, value ) 
%
% If the value is a character array (e.g. station list), then you must
% excercise some care, have a look in the code...
%
% See also rio_h5close, rio_h5create
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_h5write( h5id, name, value )

if ischar( value )
    dim   = size( value, 1 );
    len   = size( value, 2 );
    value = value';
else
    value = value(:); % ensure we have a 1-D array    
    dim   = length(value);
end
hdf5_type = rio_h5type( class( value ) );

spaceID = H5S.create_simple( 1, dim, dim );
parID   = H5P.create( 'H5P_DATASET_CREATE' );
typeID  = H5T.copy( hdf5_type );
if ischar( value )
    H5T.set_size( typeID, len );
end
dsetID     = H5D.create( h5id, name, typeID, spaceID , parID );
memspaceID = H5S.create_simple( 1, dim, [ ] );
memtypeID  = H5T.copy( hdf5_type );     
if ischar( value )
    H5T.set_size( memtypeID, len );
end
H5D.write( dsetID, memtypeID, memspaceID, spaceID, 'H5P_DEFAULT', value );

