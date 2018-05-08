% OPAQ_READCONFIG
%
% Imports an OPAQ XML configuration file
%
% Bino Maiheu, (c) VITO 2015

function cnf = opaq_readconfig( config_file )

try
    xmlFile      = xmlread( config_file );
    xmlTree      = parseChildNodes( xmlFile );
    configNodes = xmlTree.Children( strcmp( {xmlTree.Children.Name}, 'setup' ) ).Children;
catch
    error( 'OPAQ:ConfigError', 'Failed to load %s', configfile );
end

% -- parse the general parameter list
cnf = parseParamList( configNodes );

% -- parse the io section
ioNodes = configNodes( strcmp( {configNodes.Name}, 'io' ) ).Children;
cnf.io = parseParamList( ioNodes );

% -- parse the ann configuration section
annNodes = configNodes( strcmp( {configNodes.Name}, 'ann' ) ).Children;
cnf.ann = parseParamList( annNodes );

% -- parse the optimization section
optimNodes  = configNodes( strcmp( {configNodes.Name}, 'optim' ) ).Children;
cnf.optim.mode = optimNodes( strcmp( {optimNodes.Name}, 'mode' ) ).Children(1).Data;

exceedNodes = optimNodes( strcmp( {optimNodes.Name}, 'exceedance_thresholds' ) ).Children;
cnf.optim.exceedance_thresholds = parseExceedValueList( exceedNodes );

% -- parse the station network section
 try
     networkNode = xmlTree.Children( strcmp( {xmlTree.Children.Name}, 'network' ) );
     networkfile = networkNode.Attributes( strcmp( {networkNode.Attributes.Name}, 'file' ) ).Value;     
     cnf.network = opaq_readstations( networkfile );
 catch ME
     % assume we don't have the file attribute, parse the section assuming
     % the station tags are in there...
     stationNodes = networkNode.Children;
     cnf.network = [];
     for k=1:length(stationNodes)
         if ~strcmp( stationNodes(k).Name, 'station' ), continue, end;
    
         % -- add the attributes
         for l=1:length( stationNodes(k).Attributes )
             s.( stationNodes(k).Attributes(l).Name ) = stationNodes(k).Attributes(l).Value;
         end
    
         % -- now add the pollutants
         s.pollutants = strsplit( stationNodes(k).Children.Data, ',' );
        
         % -- append to the list
         cnf.network = [ cnf.network; s ];
     end
 end
 


function s = parseParamList( nodes )

s = struct();
for k=1:length(nodes)
   if strcmp(nodes(k).Name, 'param' )       
       try
           fieldName = nodes(k).Attributes( strcmp( {nodes(k).Attributes.Name}, 'name' ) ).Value;
       catch ME
           error( 'OPAQ:ConfigError', 'unable to get name for parameter' );
       end
              
       try
           fieldType = nodes(k).Attributes( strcmp( {nodes(k).Attributes.Name}, 'type' ) ).Value;
           if strcmpi( fieldType, 'char' ) || strcmpi( fieldType, 'string' )
               s.( fieldName ) = nodes(k).Children(1).Data;
           else
               s.( fieldName ) = cast( str2double( nodes(k).Children(1).Data ), fieldType );
           end
       catch ME
           % no fieldname given, assuming string
           s.( fieldName ) = nodes(k).Children(1).Data;
       end
   end   
           
end

function s = parseExceedValueList( nodes )

s = struct();
for k=1:length(nodes)
   if strcmp(nodes(k).Name, 'value' )       
       try
           fieldName = nodes(k).Attributes( strcmp( {nodes(k).Attributes.Name}, 'pol' ) ).Value;
       catch ME
           error( 'OPAQ:ConfigError', 'unable to get pollutant for exceedance value' );
       end
              
       try
           aggr = nodes(k).Attributes( strcmp( {nodes(k).Attributes.Name}, 'aggr' ) ).Value;           
       catch ME
           error( 'OPAQ:ConfigError', 'no aggregation given' );           
       end
       s.( fieldName ).( aggr ) = str2double( nodes(k).Children(1).Data );
   end   
           
end