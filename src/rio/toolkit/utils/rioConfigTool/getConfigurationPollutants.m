function list = getConfigurationPollutants( filename, cnfname )

try
   tree = xmlread(filename);
catch
   error('Failed to read XML file %s.',filename);
end

cnfEl = getTagByAttribValue( tree, 'Configuration', 'name', cnfname );

list = getTagAttribValues( cnfEl, 'Pollutant', 'name' );
