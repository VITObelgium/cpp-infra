%RIO_H5TYPE
%
% Returns the H5 type string from a matlab type
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function [ hdf5_type ] = rio_h5type( matlab_type )
% Data type for attribute
switch( matlab_type )
    case 'double'
        hdf5_type = 'H5T_NATIVE_DOUBLE';
    case 'single'
        hdf5_type = 'H5T_NATIVE_FLOAT';
    case 'int32'
        hdf5_type = 'H5T_NATIVE_INT32';
    case 'int16'
        hdf5_type = 'H5T_NATIVE_INT16';
    case 'uint8'
        hdf5_type = 'H5T_NATIVE_UINT8';
    case 'char'
        hdf5_type = 'H5T_C_S1' ;        
    otherwise
        error( 'data type %s is not implemented here', matlab_type );
end