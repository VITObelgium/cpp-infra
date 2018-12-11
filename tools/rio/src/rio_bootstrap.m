%RIO_BOOTSTRAP
% This routine creates an empty RIO folder structure at the path location
% folder are created as subfolders of the path given as argument
%  
% rio_bootstrap( basepath )
%
% The folder structure created looks like : 
%
%  <basepath>/data
%  <basepath>/stations
%  <basepath>/rio_param/spatial_corr
%  <basepath>/rio_param/trend
%  <basepath>/rio_param/stat_param
%  <basepath>/land_use
%
% Note that the pollutant subdirectories are not created by this routine. 
%
% See also rio_log, rio_init
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function rio_bootstrap( basepath )

warning( '+++ this routine is commented out and will be changed soon...' );

% rio_log( sprintf( 'bootstrapping RIO in %s', basepath ) );
% if ~exist( fullfile( basepath, 'data', '' ), 'dir' );
%     mkdir( basepath, 'data' );
% end
% if ~exist( fullfile( basepath, 'stations', '' ), 'dir' );
%     mkdir( basepath, 'stations' )
% end
% if ~exist( fullfile( basepath, 'rio_param', '' ), 'dir' );
%     mkdir( basepath, 'rio_param' )
%     mkdir( fullfile( basepath, 'rio_param', '' ), 'spatial_corr' );
%     mkdir( fullfile( basepath, 'rio_param', '' ), 'stat_param' );
%     mkdir( fullfile( basepath, 'rio_param', '' ), 'trend' );
% end
% if ~exist( fullfile( basepath, 'land_use', '' ), 'dir' );
%     mkdir( basepath, 'land_use' );
% end




