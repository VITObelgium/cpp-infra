function list = getConfigurations( filename )

try
   tree = xmlread(filename);
catch
   error('Failed to read XML file %s.',filename);
end

list = getTagAttribValues( tree, 'Configuration', 'name' );
