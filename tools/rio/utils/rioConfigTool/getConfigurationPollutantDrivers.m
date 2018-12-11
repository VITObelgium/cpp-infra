function list = getConfigurationPollutantDrivers( filename, cnfname, polname )

try
   tree = xmlread(filename);
catch
   error('Failed to read XML file %s.',filename);
end
list = {};

cnfEl = getTagByAttribValue( tree, 'Configuration', 'name', cnfname );
if ~isempty( cnfEl )
    polEl = getTagByAttribValue( cnfEl, 'Pollutant', 'name', polname );
    if ~isempty( polEl )
        list = getTagAttribValues( polEl, 'Driver', 'name' );
    end
end