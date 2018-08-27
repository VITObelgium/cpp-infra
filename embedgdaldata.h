#pragma once

namespace inf::gdal {

// Create the in memory gdal data files (call this ones in the application)
void createEmbeddedData();
void destroyEmbeddedData();

// Call these functions on every thread that needs access to the data files
void registerEmbeddedDataFileFinder();
void unregisterEmbeddedDataFileFinder();
}
