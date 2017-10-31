#pragma once

namespace infra::gdal {

// Note: The embedded data is only available on the thread that calls this function
void createEmbeddedData();
void destroyEmbeddedData();

}