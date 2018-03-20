#pragma once

#include "infra/filesystem.h"
#include "infra/geometadata.h"
#include "infra/internal/gdalinternal.h"
#include "infra/point.h"

#include <algorithm>
#include <array>
#include <complex>
#include <fmt/format.h>
#include <gdal_priv.h>
#include <gsl/span>
#include <ogr_feature.h>
#include <ogr_spatialref.h>

#include <chrono>
#include <optional>
#include <variant>

class OGRSimpleCurve;
class OGRFieldDefn;
class OGRPointIterator;
class OGRGeometryCollection;
class OGRMultiLineString;
class OGRLinearRing;

namespace infra::gdal {

using namespace std::string_literals;

using days       = std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;
using date_point = std::chrono::time_point<std::chrono::system_clock, days>;

// RAII wrapper for gdal registration
class Registration
{
public:
    Registration();
    ~Registration();
};

// Has to be instantiated on each thread that requires access to the proj.4 data
class EmbeddedDataRegistration
{
public:
    EmbeddedDataRegistration();
    ~EmbeddedDataRegistration();
};

// Free function versions of the registration handling
// Call this ones in each application that wishes to use gdal
void registerGdal();
void unregisterGdal();

// Call this on each thread that requires access to the proj.4 data
// Not needed on the thread that did the gdal registration
void registerEmbeddedData();
void unregisterEmbeddedData();

class Layer;
class Driver;

enum class RasterType
{
    Memory,
    ArcAscii,
    GeoTiff,
    Gif,
    Png,
    PcRaster,
    Unknown
};

enum class VectorType
{
    Memory,
    Csv,
    Tab,
    ShapeFile,
    Xlsx,
    Unknown
};

class CoordinateTransformer
{
public:
    CoordinateTransformer(int32_t sourceEpsg, int32_t destEpsg);

    Point<double> transform(const Point<double>& point) const;
    void transformInPlace(Point<double>& point) const;

private:
    OGRSpatialReference _sourceSRS;
    OGRSpatialReference _targetSRS;
    std::unique_ptr<OGRCoordinateTransformation> _transformer;
};

/* convenience function to convert a single point (internally creates a CoordinateTransformer)
 * Don't use this function for converting a lot of points as there is a significant overhead
 * in creating a CoordinateTransformer instance for every point
 */
Point<double> convertPointProjected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point);
Point<double> projectedToGeoGraphic(int32_t epsg, Point<double>);
std::string projectionToFriendlyName(const std::string& projection);
std::string projectionFromEpsg(int32_t epsg);
int32_t projectionToGeoEpsg(const std::string& projection);
int32_t projectionToEpsg(const std::string& projection);
std::vector<const char*> createOptionsArray(const std::vector<std::string>& driverOptions);

RasterType guessRasterTypeFromFileName(const std::string& filePath);
VectorType guessVectorTypeFromFileName(const std::string& filePath);

class Line
{
public:
    Line(OGRSimpleCurve* curve);

    int pointCount() const;
    Point<double> pointAt(int index) const;

    Point<double> startPoint();
    Point<double> endPoint();

    OGRSimpleCurve* get();

private:
    OGRSimpleCurve* _curve;
};

class LineIterator
{
public:
    LineIterator() = default;
    LineIterator(Line line);
    LineIterator(const LineIterator&) = delete;
    LineIterator(LineIterator&&)      = default;
    ~LineIterator();

    LineIterator& operator++();
    LineIterator& operator=(LineIterator&& other);
    bool operator==(const LineIterator& other) const;
    bool operator!=(const LineIterator& other) const;
    const Point<double>& operator*();
    const Point<double>* operator->();

private:
    void next();

    OGRPointIterator* _iter = nullptr;
    Point<double> _point;
};

inline LineIterator begin(const Line& line)
{
    return LineIterator(line);
}

inline LineIterator begin(Line&& line)
{
    return LineIterator(line);
}

inline LineIterator end(const Line&)
{
    return LineIterator();
}

class MultiLine
{
public:
    MultiLine(OGRMultiLineString* multiLine);

    OGRMultiLineString* get();

    int geometryCount() const;
    Line geometry(int index) const;

private:
    OGRMultiLineString* _multiLine;
};

class LinearRing : public Line
{
public:
    LinearRing(OGRLinearRing* ring);

    OGRLinearRing* get();

private:
    OGRLinearRing* _ring;
};

class Polygon
{
public:
    Polygon(OGRPolygon* poly);

    LinearRing exteriorRing();
    LinearRing interiorRing(int index);

    OGRPolygon* get();

private:
    OGRPolygon* _poly;
};

using Geometry = std::variant<std::monostate, Point<double>, Line, MultiLine, Polygon>;
using Field    = std::variant<int32_t, int64_t, double, std::string_view>;

class FieldDefinition
{
public:
    FieldDefinition() = default;
    FieldDefinition(const char* name, const std::type_info& typeInfo);
    FieldDefinition(OGRFieldDefn* def);
    ~FieldDefinition();
    std::string_view name() const;
    const std::type_info& type() const;

    OGRFieldDefn* get() noexcept;

private:
    bool _hasOwnerShip = false;
    OGRFieldDefn* _def = nullptr;
};

class FeatureDefinition
{
public:
    FeatureDefinition(OGRFeatureDefn* def);
    ~FeatureDefinition();
    std::string_view name() const;

    int fieldCount() const;
    int fieldIndex(std::string_view name) const;
    FieldDefinition fieldDefinition(int index) const;

    OGRFeatureDefn* get() noexcept;

private:
    bool _hasOwnerShip;
    OGRFeatureDefn* _def;
};

class Feature
{
public:
    Feature(Layer& layer);
    explicit Feature(OGRFeature* feature);
    Feature(const Feature&) = delete;
    Feature(Feature&&);
    ~Feature();

    Feature& operator=(const Feature&) = delete;
    Feature& operator                  =(Feature&&);

    OGRFeature* get();
    const OGRFeature* get() const;

    Geometry geometry();
    const Geometry geometry() const;

    int fieldCount() const;
    int fieldIndex(std::string_view name) const;
    FieldDefinition fieldDefinition(int index) const;

    Field getField(int index) const noexcept;

    template <typename T>
    T getFieldAs(int index) const;

    template <typename T>
    T getFieldAs(std::string_view name) const;

    template <typename T>
    void setField(std::string_view name, const T& value)
    {
        _feature->SetField(name.data(), value);
    }

    bool operator==(const Feature& other) const;

private:
    OGRFeature* _feature;
};

class Layer
{
public:
    explicit Layer(OGRLayer* layer);
    Layer(const Layer&);
    Layer(Layer&&);
    ~Layer();

    Layer& operator=(Layer&&) = default;

    int64_t featureCount() const;
    Feature feature(int64_t index) const;

    int fieldIndex(std::string_view name) const;
    void setSpatialFilter(Point<double> point);

    void createField(FieldDefinition& field);
    void createFeature(Feature& feature);

    FeatureDefinition layerDefinition() const;

    const char* name() const;
    OGRLayer* get();
    const OGRLayer* get() const;

private:
    OGRLayer* _layer;
};

// Iteration is not thread safe!
// Do not iterate simultaneously from different threads.
class LayerIterator
{
public:
    LayerIterator();
    LayerIterator(Layer layer);
    LayerIterator(const LayerIterator&) = delete;
    LayerIterator(LayerIterator&&)      = default;

    LayerIterator& operator++();
    LayerIterator& operator=(LayerIterator&& other);
    bool operator==(const LayerIterator& other) const;
    bool operator!=(const LayerIterator& other) const;
    const Feature& operator*();
    const Feature* operator->();

private:
    void next();

    Layer _layer;
    Feature _currentFeature;
};

// support for range based for loops
inline LayerIterator begin(Layer& layer)
{
    return LayerIterator(layer);
}

inline LayerIterator begin(Layer&& layer)
{
    return LayerIterator(layer);
}

inline LayerIterator end(const infra::gdal::Layer& /*layer*/)
{
    return LayerIterator();
}

class FeatureIterator
{
public:
    FeatureIterator(int _count);
    FeatureIterator(const Feature& feature);
    FeatureIterator(const FeatureIterator&) = delete;
    FeatureIterator(FeatureIterator&&)      = default;

    FeatureIterator& operator++();
    FeatureIterator& operator=(FeatureIterator&& other) = default;
    bool operator==(const FeatureIterator& other) const;
    bool operator!=(const FeatureIterator& other) const;
    const Field& operator*();
    const Field* operator->();

private:
    void next();

    const Feature* _feature = nullptr;
    int _fieldCount         = 0;
    int _currentFieldIndex  = 0;
    Field _currentField;
};

// support for range based for loops
inline FeatureIterator begin(const Feature& feat)
{
    return FeatureIterator(feat);
}

inline FeatureIterator begin(Feature&& feat)
{
    return FeatureIterator(feat);
}

inline FeatureIterator end(const Feature& feat)
{
    return FeatureIterator(feat.fieldCount());
}

class FeatureDefinitionIterator
{
public:
    FeatureDefinitionIterator(int fieldCount);
    FeatureDefinitionIterator(const FeatureDefinition& featureDef);
    FeatureDefinitionIterator(const FeatureDefinitionIterator&) = delete;
    FeatureDefinitionIterator(FeatureDefinitionIterator&&)      = default;

    FeatureDefinitionIterator& operator++();
    FeatureDefinitionIterator& operator=(FeatureDefinitionIterator&& other) = default;
    bool operator==(const FeatureDefinitionIterator& other) const;
    bool operator!=(const FeatureDefinitionIterator& other) const;
    const FieldDefinition& operator*();
    const FieldDefinition* operator->();

private:
    void next();

    const FeatureDefinition* _featureDef = nullptr;
    int _fieldCount                      = 0;
    int _currentFieldIndex               = 0;
    FieldDefinition _currentField;
};

// support for range based for loops
inline FeatureDefinitionIterator begin(const FeatureDefinition& featDef)
{
    return FeatureDefinitionIterator(featDef);
}

inline FeatureDefinitionIterator begin(FeatureDefinition&& featDef)
{
    return FeatureDefinitionIterator(featDef);
}

inline FeatureDefinitionIterator end(const FeatureDefinition& featDef)
{
    return FeatureDefinitionIterator(featDef.fieldCount());
}

class RasterBand
{
public:
    RasterBand(GDALRasterBand* ptr);

    GDALRasterBand* get();
    const GDALRasterBand* get() const;

private:
    GDALRasterBand* _band;
};

class DataSet
{
public:
    // This can only be used for raster types
    static DataSet createRaster(const std::string& filePath, const std::vector<std::string>& driverOpts = {});
    static DataSet createRaster(const std::string& filePath, RasterType type, const std::vector<std::string>& driverOpts = {});
    // if you know the type of the dataset, this will be faster as not all drivers are queried
    static DataSet createVector(const std::string& filePath, const std::vector<std::string>& driverOptions = {});
    static DataSet createVector(const std::string& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

    DataSet() = default;
    explicit DataSet(GDALDataset* ptr) noexcept;

    DataSet(DataSet&&);
    ~DataSet() noexcept;

    DataSet& operator=(DataSet&&);

    int32_t rasterCount() const;
    int32_t layerCount() const;

    int32_t rasterXSize() const;
    int32_t rasterYSize() const;
    std::array<double, 6> geoTransform() const;
    void setGeoTransform(const std::array<double, 6>& trans);

    std::optional<double> noDataValue(int bandNr) const;
    void setNoDataValue(int bandNr, std::optional<double> value) const;

    void setColorTable(int bandNr, const GDALColorTable* ct);

    std::string projection() const;
    void setProjection(const std::string& proj);

    void setMetadata(const std::string& name, const std::string& value, const std::string& domain = "");

    Layer getLayer(int index);
    Layer createLayer(const std::string& name, const std::vector<std::string>& driverOptions = {});

    RasterBand rasterBand(int index) const;

    GDALDataType getBandDataType(int index) const;

    template <typename T>
    void readRasterData(int band, int xOff, int yOff, int xSize, int ySize, T* pData, int bufXSize, int bufYSize, int pixelSize = 0, int lineSize = 0) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        checkError(bandPtr->RasterIO(GF_Read, xOff, yOff, xSize, ySize, pData, bufXSize, bufYSize, TypeResolve<T>::value, pixelSize, lineSize),
                   "Failed to read raster data");
    }

    template <typename T>
    void writeRasterData(int band, int xOff, int yOff, int xSize, int ySize, const T* pData, int bufXSize, int bufYSize) const
    {
        auto* bandPtr = _ptr->GetRasterBand(band);
        auto* dataPtr = const_cast<void*>(static_cast<const void*>(pData));
        checkError(bandPtr->RasterIO(GF_Write, xOff, yOff, xSize, ySize, dataPtr, bufXSize, bufYSize, TypeResolve<T>::value, 0, 0),
                   "Failed to write raster data");
    }

    template <typename T>
    void addBand(T* data)
    {
        // convert the data pointer to a string
        std::array<char, 32> buf;
        auto writtenCharacters = CPLPrintPointer(buf.data(), const_cast<void*>(reinterpret_cast<const void*>(data)), static_cast<int>(buf.size()));
        buf[writtenCharacters] = 0;

        auto pointerString = fmt::format("DATAPOINTER={}", buf.data());
        std::array<const char*, 2> options{{pointerString.c_str(), nullptr}};

        _ptr->AddBand(TypeResolve<typename std::remove_cv<T>::type>::value, options.empty() ? nullptr : const_cast<char**>(options.data()));
    }

    template <typename T>
    void addBand(T* data, GDALColorInterp colorInterp)
    {
        addBand<T>(data);
        auto* band = _ptr->GetRasterBand(_ptr->GetRasterCount());
        band->SetColorInterpretation(colorInterp);
    }

    GDALDataset* get() const;
    Driver driver();

private:
    static GDALDataset* create(const std::string& filename,
                               unsigned int openFlags,
                               const char* const* drivers,
                               const std::vector<std::string>& driverOpts);
    explicit DataSet(const std::string& filename);

    GDALDataset* _ptr = nullptr;
};

class Driver
{
public:
    static Driver create(RasterType);
    static Driver create(VectorType);
    static Driver create(const std::string& filename);

    explicit Driver(GDALDriver& driver);

    template <typename T>
    DataSet createDataSet(uint32_t rows, uint32_t cols, uint32_t numBands, const std::string& filename)
    {
        return DataSet(checkPointer(_driver.Create(filename.c_str(), cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    template <typename T>
    DataSet createDataSetCopy(const DataSet& reference, const std::string& filename, const std::vector<std::string>& driverOptions = {})
    {
        auto options = createOptionsArray(driverOptions);
        return DataSet(checkPointer(_driver.CreateCopy(
                                        filename.c_str(),
                                        reference.get(),
                                        FALSE,
                                        options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
                                        nullptr,
                                        nullptr),
                                    "Failed to create data set copy"));
    }

    RasterType rasterType() const;
    VectorType vectorType() const;

private:
    GDALDriver& _driver;
};

class MemoryFile
{
public:
    MemoryFile(std::string path, gsl::span<const uint8_t> dataBuffer);
    ~MemoryFile();

    const std::string& path() const;

private:
    const std::string _path;
    VSILFILE* _ptr;
};

infra::GeoMetadata readMetadataFromDataset(const gdal::DataSet& dataSet);
}
