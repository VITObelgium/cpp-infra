% OPAQ_READSTATIONS
%
% Imports an OPAQ XML network file
%
% Bino Maiheu, (c) VITO 2015

function st_info = opaq_readstations( network_file )

try
    xmlFile      = xmlread( network_file );
    xmlTree      = parseChildNodes( xmlFile );
    stationNodes = xmlTree.Children( strcmp( {xmlTree.Children.Name}, 'network' ) ).Children;
catch
    error( 'Failed to load %s', network_file );
end

st_info = [];
for k=1:length(stationNodes)
    if ~strcmp( stationNodes(k).Name, 'station' ), continue, end;    
    
    % -- add the attributes
    for l=1:length( stationNodes(k).Attributes )
        s.( stationNodes(k).Attributes(l).Name ) = stationNodes(k).Attributes(l).Value;
    end
    
    % -- now add the pollutants
    s.pollutants = strsplit( stationNodes(k).Children.Data, ',' );
        
    % -- append to the list
    st_info = [ st_info; s ];
end

