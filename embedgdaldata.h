#pragma once

namespace inf::gdal {

// Create the in memory gdal data files (call this ones in the application)
void create_embedded_data();
void destroy_embedded_data();

// Call these functions on every thread that needs access to the data files
void register_embedded_data_file_finder();
void unregister_embedded_data_file_finder();
}
