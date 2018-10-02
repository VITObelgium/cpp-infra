%RIO_H5CLOSE
%
% Closes a RIO hdf5 output file
% 
%  rio_h5close( fh )
%
% The routine should recieve the hdf5 filehandle structure created by
% the rio_h5create routine.
%
% See also rio_init, rio_h5create
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_h5close( fh )

rio_h5attr( fh.h5fID, 'run_finish', datestr( now, 31 ) );

fprintf( 'Closing file %s\n', fh.fname );
H5F.close ( fh.h5fID );
end