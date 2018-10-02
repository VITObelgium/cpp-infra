%RIO_LOG
% Generates a RIO logging message to see what has been going on, 
% file is written to the 'librio.log' file in the working folder.
% 
%  rio_log(  msg )
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_log( msg )

fid = fopen( 'librio.log', 'at' );
if fid == -1
    error( 'rio_log:: unable to open log file' );
end

fprintf( fid, '[%s] %s\n', datestr(now), strtrim( msg ) );

fclose( fid );
