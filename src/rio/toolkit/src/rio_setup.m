%RIO_SETUP
% Parses the RIO XML setup file and loads the matching configuration into 
% the setup structure. This routine replaces the rio_init.
%
%  setup = rio_setup( xmlfilename, configuration, pollutant, agg_time, proxy, grid )
%  setup = rio_setup( xmlfilename, configuration, pollutant, agg_time, proxy )
%
%
% Changelog
%   - 2016-08-30 : adjusted to conform to the fortran XML format... 
%
% See also rio rio_init
%
% RIO (c) VITO/IRCEL 2004-2016 
% Jef Hooybergs, Stijn Janssen, Nele Veldeman, Bino Maiheu

function cnf = rio_setup( filename, config_name, pol, agg_timestr, gis_type, varargin )

% Default grid type, not needed for the validation, so optional argument
% too...
grid_type = '';
if nargin > 5
  grid_type = varargin{1};
end


% Initialise the configuration structure
cnf = struct( 'version', rio_version );
xml = parseXML(filename);
xml = xml( strcmp( {xml.Name}, 'RIO' ) );
if isempty( xml ), error( 'Unable to find RIO root element in setup file\n' ); end;

if isempty( grid_type )
  rio_log( sprintf( 'RIO toolkit version %d.%d for %s-%s, %s (NO GRID DEFINED)', ...
    cnf.version.major, cnf.version.minor, pol, agg_timestr, gis_type ) );
else
  rio_log( sprintf( 'RIO toolkit version %d.%d for %s-%s, %s on %s grid', ...
    cnf.version.major, cnf.version.minor, pol, agg_timestr, gis_type, grid_type ) );
end
cnf.errcode   = 0;
cnf.errmsg    = '';

% Set the base configuration
cnf.pol        = pol;
cnf.pol_xx     = pol; % overwrite under certain cases, this is used !
cnf.at_lb      = agg_timestr;
cnf.gis_type   = gis_type;
cnf.grid_type  = grid_type;

% need a better solution for these guys...
cnf.deployment       = '';
cnf.ipol_mode        = '';
cnf.missing_value    = -999;
cnf.outputXY         = true; 
cnf.select_weekpart  = true;   % distinguish between week/weekend <-- should be in the Configuration
cnf.verbose          = true;   % should be command line option

%% Aggregation time labelling
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


%% Some flags, which are filled by other routines
cnf.have_stations    = false;  % true if station info loaded
cnf.have_grid        = false;  % true if grid inof loaded
cnf.have_db          = false;  % will become true if a historic database is loaded
cnf.have_spcorr      = false;  % spatial correlations loaded
cnf.have_trend       = false;  % trend info loaded
cnf.have_stats       = false;  % we have long term statistics
cnf.output_init      = false;  % set to true after the first rio_output call, to clear persistent vars

%% Define some function handles which will be used in the model
cnf.plane_lin = @(p,x)  p(1)*x(:,1)    +  p(2)*x(:,2)    + p(3);
cnf.plane_2nd = @(p,x)  p(1)*x(:,1).^2 +  p(2)*x(:,2).^2 + p(3)*x(:,1).*x(:,2)    + p(4)*x(:,1)       + p(5)*x(:,2)          + p(6);
cnf.plane_4th = @(p,x)  p(1)*x(:,1).^4 +  p(2)*x(:,2).^4 + p(3)*x(:,1).^3.*x(:,2) + p(4)*x(:,1).*x(:,2).^3 + p(5)*x(:,1).^2.*x(:,2).^2 + ...
    p(6)*x(:,1).^3 +  p(7)*x(:,2).^3 + p(8)*x(:,1).^2.*x(:,2) + p(9)*x(:,1).*x(:,2).^2 + ...
    p(10)*x(:,1).^2 + p(11)*x(:,2).^2 + p(12)*x(:,1).*x(:,2)  + ...
    p(13)*x(:,1)    + p(14)*x(:,2)    + p(15);


%% Set the paths for the requested configuration from the XML file
found_config = false;
idx = find( strcmp( {xml.Children.Name}, 'Configuration'));
for i=idx;    
    if strcmp( getAttrib( xml.Children(i), 'name' ), config_name )
        cnf.configName   = config_name;
        cnf.outputPath   = fullfile( getAttrib(xml.Children(i), 'output' ),  sprintf( '%s', cnf.pol ), '' );
        cnf.dbasePath    = fullfile( getAttrib(xml.Children(i), 'dbase' ),   sprintf( '%s', cnf.pol ), '' );
        cnf.stationsPath = fullfile( getAttrib(xml.Children(i), 'stations'), sprintf( '%s', cnf.pol ), '' );
        cnf.driversPath  = fullfile( getAttrib(xml.Children(i), 'drivers'),  sprintf( '%s', cnf.pol ), '' );        
        cnf.paramPath    = fullfile( getAttrib(xml.Children(i), 'param' ),   sprintf( '%s', cnf.pol ), '' );
        found_config     = true;
        cnfNode          = xml.Children(i);
    end
end
if ~found_config, error( 'No matching configuration found : %s...', config_name ); end;

% Checking some paths
if ~exist( cnf.outputPath, 'dir' )
    warning( '+++ creating outputPath %s\n', cnf.outputPath );
    mkdir( cnf.outputPath )
end
if ~exist( cnf.dbasePath, 'dir' ), warning( '+++ dbasePath %s not found\n', cnf.dbasePath ); end;
if ~exist( cnf.stationsPath, 'dir' ), warning( '+++ stationsPath %s not found\n', cnf.stationsPath ); end;
if ~exist( cnf.driversPath, 'dir' ), warning( '+++ driversPath %s not found\n', cnf.driversPath ); end;
if ~exist( cnf.paramPath, 'dir' ), warning( '+++ paramPath %s not found\n', cnf.paramPath ); end;

% Print the configuration paths
fprintf( 'RIO Configuration : %s\n', config_name );
fprintf( '- dbasePath    : %s\n', cnf.dbasePath );
fprintf( '- stationsPath : %s\n', cnf.stationsPath );
fprintf( '- driversPath  : %s\n', cnf.driversPath );
fprintf( '- paramPath    : %s\n', cnf.paramPath );
fprintf( '- outputPath   : %s\n', cnf.outputPath );

%% Get the requested pollutant in the configuration
polNodes = cnfNode.Children( strcmp( {cnfNode.Children.Name}, 'Pollutant' ) );
polNode = nodeWithAttribute( polNodes, 'name', pol );
if isempty( polNode )
    error( 'Pollutant %s not supported in configuration : %s ... ', pol, config_name );
end    

%% Get the requested station list in the configuration...
cnf.stationList = {};
stNodes = polNode.Children( strcmp( {polNode.Children.Name}, 'Stationlist' ) );
if numel(stNodes) == 1
  st_list  = strtrim( stNodes.Children.Data );
  st_list( isspace(st_list) ) = [];
  C = textscan( st_list, '%s', 'delimiter', ',');
  cnf.stationList = C{1};
  if strcmpi( cnf.stationList{1}, 'all' )    
      cnf.stationList = {};
  else
      warning( '+++ XML based stationlist defined !' );
  end
end

%% Get the trend parameters... 
% -- get the spatial driver node...
drvNodes = polNode.Children( strcmp( { polNode.Children.Name }, 'Driver' ) );
drvNode = nodeWithAttribute( drvNodes, 'name', gis_type );
if isempty( drvNode )
    error( 'Driver %s not supported in configuration : %s, pollutant : %s... ', ...
        gis_type, config_name, pol );
end

% -- get the selected aggregation nodes...
aggNodes = drvNode.Children( strcmp( { drvNode.Children.Name}, 'Aggregation' ) );
aggNode = nodeWithAttribute( aggNodes, 'name', agg_timestr );
if isempty( aggNode )
    error( 'Aggregation time %s not supported in configuration : %s, pollutant : %s, driver : %s... ', ...
        agg_timestr, gis_type, config_name, pol, gis_type );
end 

% -- lookup the trend parameters
cnf.avgTrend.indic_lo  = 0; % default value for indic_lo
cnf.stdTrend.indic_lo  = 0; % default value         
cnf.avgTrend.indic_hi  = inf; % default value for indic_hi
cnf.stdTrend.indic_hi  = inf; % default value
try
    avgTrendNode = aggNode.Children( strcmp( { aggNode.Children.Name }, 'avgtrend' ) );
    stdTrendNode = aggNode.Children( strcmp( { aggNode.Children.Name }, 'stdtrend' ) );
    
    cnf.avgTrend.type      = getAttrib( avgTrendNode, 'type');
    cnf.avgTrend.ref_level = str2double( getAttrib( avgTrendNode, 'ref_level') );
    cnf.avgTrend.indic_lo  = str2double( getAttrib( avgTrendNode, 'indic_lo') );
    cnf.avgTrend.indic_hi  = str2double( getAttrib( avgTrendNode, 'indic_hi') );
   
    cnf.stdTrend.type = getAttrib( stdTrendNode, 'type');
    cnf.stdTrend.ref_level = str2double( getAttrib( stdTrendNode, 'ref_level') );
    cnf.stdTrend.indic_lo  = str2double( getAttrib( stdTrendNode, 'indic_lo') );
    cnf.stdTrend.indic_hi  = str2double( getAttrib( stdTrendNode, 'indic_hi') );
   
catch ME
    error( 'Error parsing the trend function configuration...' );
end

%% Read the detection limit and scale factor... 
cnf.detection_limit = 1.0;
tmp = polNode.Children( strcmp( { polNode.Children.Name }, 'DetectionLimit' ) );
if numel(tmp) == 1
    cnf.detection_limit = str2double( tmp(1).Children.Data );    
end
fprintf( 'Pollutant detection limit : %f\n', cnf.detection_limit );

try
    cnf.scale_factor = str2double( getAttrib( polNode, 'scale' ) );
catch  
    cnf.scale_factor = 1.0; % default scale factor
end
fprintf( 'Pollutant I/O scale factor to : %f\n', cnf.scale_factor );


%% Get the boolean (for now) options from the XML file...
cnf.Option.logtrans = false;   % use log transformation 
idx = find( strcmp( { cnfNode.Children.Name}, 'Option' ) );
for i=idx
  fld = getAttrib( cnfNode.Children(i), 'name' );
  if isfield( cnf.Option, fld )
    val = cnfNode.Children(i).Children(1).Data;
    switch( lower(val) )
      case 'true'
        cnf.Option.(fld) = true;
    end        
  else
    warning( 'field %s not defined in cnf.Options, skipping\n', fld );
  end
end


function x = getAttrib( xml, name )
x = xml.Attributes( strcmp( {xml.Attributes.Name}, name) ).Value;

function theStruct = parseXML(filename)
% PARSEXML Convert XML file to a MATLAB structure.
try
   tree = xmlread(filename);
catch
   error('Failed to read XML file %s.',filename);
end

% Recurse over child nodes. This could run into problems 
% with very deeply nested trees.
try
   theStruct = parseChildNodes(tree);
catch
   error('Unable to parse XML file %s.',filename);
end


% ----- Subfunction PARSECHILDNODES -----
function children = parseChildNodes(theNode)
% Recurse over node children.
children = [];
if theNode.hasChildNodes
   childNodes = theNode.getChildNodes;
   numChildNodes = childNodes.getLength;
   allocCell = cell(1, numChildNodes);

   children = struct(             ...
      'Name', allocCell, 'Attributes', allocCell,    ...
      'Data', allocCell, 'Children', allocCell);

    for count = 1:numChildNodes
        theChild = childNodes.item(count-1);
        children(count) = makeStructFromNode(theChild);
    end
end

% ----- Subfunction MAKESTRUCTFROMNODE -----
function nodeStruct = makeStructFromNode(theNode)
% Create structure of node info.

nodeStruct = struct(                        ...
   'Name', char(theNode.getNodeName),       ...
   'Attributes', parseAttributes(theNode),  ...
   'Data', '',                              ...
   'Children', parseChildNodes(theNode));

if any(strcmp(methods(theNode), 'getData'))
   nodeStruct.Data = char(theNode.getData); 
else
   nodeStruct.Data = '';
end

% ----- Subfunction PARSEATTRIBUTES -----
function attributes = parseAttributes(theNode)
% Create attributes structure.

attributes = [];
if theNode.hasAttributes
   theAttributes = theNode.getAttributes;
   numAttributes = theAttributes.getLength;
   allocCell = cell(1, numAttributes);
   attributes = struct('Name', allocCell, 'Value', ...
                       allocCell);

   for count = 1:numAttributes
      attrib = theAttributes.item(count-1);
      attributes(count).Name = char(attrib.getName);
      attributes(count).Value = char(attrib.getValue);
   end
end

function node = nodeWithAttribute( nodeList, attrName, attrValue )
node = [];
for k=1:numel( nodeList )
    if strcmp( nodeList(k).Attributes(strcmp( {nodeList(k).Attributes.Name}, attrName ) ).Value, attrValue )
        node = nodeList(k);
    end
end

