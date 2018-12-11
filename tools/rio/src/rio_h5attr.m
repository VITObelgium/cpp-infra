%RIO_H5ATTR
%
% Writes a matlab variable to a HDF5 attribute associated with the 
% given H5id. Type handling is done automatic
% 
%  fh = rio_h5attr( h5id, name, value )
%
% See also rio_init, rio_h5create, rio_h5close, rio_h5append
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_h5attr( h5id, name, value )

% dataspace for scalar attribute
s = H5S.create( 'H5S_SCALAR' );

hdf5_type = rio_h5type( class( value ) );
t = H5T.copy( hdf5_type );
if strcmp( hdf5_type, 'H5T_C_S1' )
    H5T.set_size( t, length( value ) );
end

a = H5A.create( h5id, name, t, s, 'H5P_DEFAULT' );
H5A.write( a, 'H5ML_DEFAULT', value );
H5A.close( a );
H5T.close( t );
H5S.close( s );


