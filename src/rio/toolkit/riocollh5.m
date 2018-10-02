% collects the hdf5 datasets and writs a standard rio h5 dataset

list = dir( 'nh3/*.h5' );

my_grid_value     = [];
my_grid_error     = [];
my_stations_value = [];
my_time_day       = [];
my_time_month     = [];

for i = 1:numel(list)
    my_grid_value     = [ my_grid_value; h5read( fullfile( 'nh3', list(i).name ), '/grid/value' ) ];
    my_grid_error     = [ my_grid_error; h5read( fullfile( 'nh3', list(i).name ), '/grid/error' ) ];
    my_stations_value = [ my_stations_value; h5read( fullfile( 'nh3', list(i).name ), '/stations/value' ) ];
    my_time_day       = [ my_time_day; h5read( fullfile( 'nh3', list(i).name ), '/time/day' ) ];
    my_time_month     = [ my_time_month; h5read( fullfile( 'nh3', list(i).name ), '/time/month' ) ];
    my_time_year      = [ my_time_year; h5read( fullfile( 'nh3', list(i).name ), '/time/year' ) ];        
end

% now output 
template = 'rio_nh3_wk_edp4_1x1_20120101-20121231.h5';
copyfile( 'rio_nh3_wk_edp4_1x1_template.h5', template );


h5create( template, '/grid/value', size(my_grid_value), 'ChunkSize', [ 1 size(my_grid_value,2) ], 'DataType', 'single' );
h5write( template, '/grid/value', my_grid_value );
h5writeatt( template, '/grid/value', 'scale_factor', 1. );
h5writeatt( template, '/grid/value', 'units', 'ug/m3' );
h5writeatt( template, '/grid/value', 'missing_value', int32(-9999) );

h5create( template, '/grid/error', size(my_grid_error), 'ChunkSize', [ 1 size(my_grid_error,2) ], 'DataType', 'single' );
h5write( template, '/grid/error', my_grid_error );
h5writeatt( template, '/grid/error', 'scale_factor', 1. );
h5writeatt( template, '/grid/error', 'units', 'ug/m3' );
h5writeatt( template, '/grid/error', 'missing_value', int32(-9999) );

h5create( template, '/stations/value', size(my_stations_value), 'ChunkSize', [ 1 size(my_stations_value,2) ], 'DataType', 'single' );
h5write( template, '/stations/value', my_stations_value );
h5writeatt( template, '/stations/value', 'scale_factor', 1. );
h5writeatt( template, '/stations/value', 'units', 'ug/m3' );
h5writeatt( template, '/stations/value', 'missing_value', int32(-9999) );

h5create( template, '/time/year',  size(my_time_year'), 'DataType', 'int32' );
h5create( template, '/time/month', size(my_time_month'), 'DataType', 'int32'  );
h5create( template, '/time/day',   size(my_time_day'), 'DataType', 'int32'  );
h5write( template, '/time/year',  int32(my_time_year') );
h5write( template, '/time/month', int32(my_time_month') );
h5write( template, '/time/day',   int32(my_time_day') );

% adjust some attributes
h5writeatt( template, '/', '__SPECIAL_NOTE__', 'Created in postproduction with riocollh5 !!' );
