%RIO_INIT
% This routine initialises the RIO library for running, it returns a
% configuration structure needed by the rio library routines.
% In the input parameters, the user needs to specify the basepath 
% for the RIO installation (where it will look for the station & trend 
% files etc) along with the pollutant for which to run the interpolation.
%
% cnf = rio_init( base, pol, agg_time, gis_type, grid_type [, ipol_mode ] )
%
% Input arguments
%  - base           : the base path
%  - pol            : pollutant requested
%  - agg_timestr    : pollutant aggregation time label (m1, m8, da, 1h)
%  - gis_type       : spatial driver name (proxy, or gis_type)
%  - grid_type      : the grid type 
% Optional 
%   - ipol_mode     : interpolation mode (RIO, OrdKrig, IDW)
%
% A number of cnfuration parameters are set in this routine as well,
% they can be altered later on by the user :
%  cnf.base         : base dir of the deployment, basically the top level
%                     folder of the entire rio_param, stations, land_use etc 
%                     folder structure
%  cnf.pol          : requested pollutant name
%  cnf.pol_xx       : internal pollutant name, normally the same as above,
%                     but e.g. in case of O3s it's not...
%  cnf.at_lb        : aggregation time label : m1, m8, da, 1h
%  cnf.agg_time_loc : aggregation time locator in the data arrays
%  cnf.gis_type     : the proxy time : CorineID, AOD etc...
%  cnf.grid_type    : the grid type, can be 1x1, 4x4, belEUROS etc...
%  cnf.ipol_mode    : interpolation mode : RIO (default), OrdKrig, IDW
%                     etc...
%  cnf.deployment   : a string indicating in which environment the library
%                     is deployed, this can be used to set certain 
%                     deployment specific things in the library itself, 
%                     which would otherwise involve too much re-coding.
%  cnf.verbose      : regulates the verbosity of output for the RIO
%                     routines
%
%  cnf.select_weekpart  : default is true, distinguish between week/weekend 
%                         for trend functions, if false, we'll load the '_all_'
%                         trendfunctions, otherwise we'll use the
%                         '_week_' / '_weekend_' ones
%
%  cnf.missing_value    : a missing value for the output of RIO
%  cnf.have_stations    : true if station info loaded
%  cnf.have_db          : true if a historic database is loaded
%  cnf.have_spcorr      : true if spatial correlations loaded
%  cnf.have_trend       : true if trend info loaded
%  cnf.have_stats       : true if long term statistics loaded
%                    
%  cnf.outpath          : output path (default <base>/results/<pol>_val)
%  cnf.outputXY         : store XY coordinates with the grid cell,
%                            default is true
%
%  Changelog
%   - 2010.04.09 : Created (bino.maiheu@vito.be)
%   - 2010.08.04 : Treat properly the output path, currently it is
%       automatically created, but needs to be user specifiable, i've updated
%       the routines rio_output and rio_outputfilenames so they can ingest a
%       user specified path as well, but not the rio_init yet...
%
% Examples
%   cnf = rio_init( '.', 'pm10', 'da', 'CorineID', '4x4' )   
%
%         This configures the RIO model for running with PM10 on daily
%         averages and using the CorineID ( ordinary beta) as a land use
%         parameter on the 4x4 grid. RIO expects the folder structure in
%         the current path.
%
%   cnf = rio_init( '.', 'no2', '1h', 'Corine_double_beta', '1x1' )
%       
%         This configures the model to run hourly interpolations, using the
%         double beta driver on the 1x1 km grid.
%
% See also rio_log, rio_version
%
% RIO (c) VITO/IRCEL 2004-2011 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function cnf = rio_init( base, pol, agg_timestr, gis_type, grid_type, varargin )

% Init configuration structure
cnf         = struct();
cnf.version = rio_version;
rio_log( sprintf( 'Init RIO version %d.%d for %s-%s, %s on %s grid', ...
    cnf.version.major, cnf.version.minor, pol, agg_timestr, gis_type, grid_type ) );
cnf.errcode   = 0;
cnf.errmsg    = '';

cnf.base       = base;

cnf.pol        = pol;
cnf.pol_xx     = pol; % overwrite under certain cases, this is used !
cnf.at_lb      = agg_timestr;
cnf.gis_type   = gis_type;
cnf.grid_type  = grid_type;

if nargin > 5
    cnf.ipol_mode  = varargin{1};
else
    cnf.ipol_mode  = 'RIO';
end

cnf.deployment = ''; % set empty deployment by default....

% % Checking basepath filestructure : station files, parameter files, proxy
% % data and concentration database files...
% if ~exist( fullfile( cnf.base, 'stations', '' ), 'dir' ) || ...
%         ~exist( fullfile( cnf.base, 'rio_param', 'spatial_corr', '' ), 'dir' ) || ...
%         ~exist( fullfile( cnf.base, 'rio_param', 'stat_param', '' ), 'dir' ) || ...
%         ~exist( fullfile( cnf.base, 'rio_param', 'trend', '' ), 'dir' ) || ...
%         ~exist( fullfile( cnf.base, 'land_use', '' ), 'dir' ) || ...
%         ~exist( fullfile( cnf.base, 'data', '' ), 'dir' )
%     cnf.errmsg  = sprintf( 'invalid base path structure in %s', cnf.base );
%     cnf.errcode = 1;
%     return;
% end

% Set agg_time from the label
switch cnf.at_lb
    case 'm1'
        cnf.agg_time = 1;
    case 'm8'
        cnf.agg_time = 2;
    case 'da'
        cnf.agg_time = 3;
    case '1h'
        cnf.agg_time = 4;
    otherwise
        cnf.errmsg  =  sprintf( 'invalid aggregation time label %s', cnf.at_lb );
        cnf.errcode = 2;
        return;
end

% Set aggregation time locator in trend & statistics arrays...
if cnf.agg_time <= 3
    cnf.agg_time_loc = cnf.agg_time + 1;
elseif cnf.agg_time == 4
    cnf.agg_time_loc = ( 1:24 ) + 1;
end

% RIO output missing value
cnf.missing_value = -999;

% Set some flags, which are filled by other routines
cnf.have_stations    = false;  % true if station info loaded
cnf.have_grid        = false;  % true if grid inof loaded
cnf.have_db          = false;  % will become true if a historic database is loaded
cnf.have_spcorr      = false;  % spatial correlations loaded
cnf.have_trend       = false;  % trend info loaded
cnf.have_stats       = false;  % we have long term statistics
cnf.select_weekpart  = true;   % distinguish between week/weekend
cnf.verbose          = true;

% Define some function handles which will be used in the model
cnf.plane_lin = @(p,x)  p(1)*x(:,1)    +  p(2)*x(:,2)    + p(3);
cnf.plane_2nd = @(p,x)  p(1)*x(:,1).^2 +  p(2)*x(:,2).^2 + p(3)*x(:,1).*x(:,2)    + p(4)*x(:,1)       + p(5)*x(:,2)          + p(6);
cnf.plane_4th = @(p,x)  p(1)*x(:,1).^4 +  p(2)*x(:,2).^4 + p(3)*x(:,1).^3.*x(:,2) + p(4)*x(:,1).*x(:,2).^3 + p(5)*x(:,1).^2.*x(:,2).^2 + ...
    p(6)*x(:,1).^3 +  p(7)*x(:,2).^3 + p(8)*x(:,1).^2.*x(:,2) + p(9)*x(:,1).*x(:,2).^2 + ...
    p(10)*x(:,1).^2 + p(11)*x(:,2).^2 + p(12)*x(:,1).*x(:,2)  + ...
    p(13)*x(:,1)    + p(14)*x(:,2)    + p(15);

% Output options, default output location : absolute path !
cnf.outpath = fullfile( cnf.base, 'results', sprintf( '%s_val', cnf.pol ) );
if ~exist( cnf.outpath, 'dir' )
    mkdir( cnf.outpath )
end
cnf.outputXY = true; 
                
end
