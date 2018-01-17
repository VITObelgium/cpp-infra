#pragma once

#include "infra/filesystem.h"
#include "infra/internal/gdalinternal.h"
#include "infra/point.h"

#include <algorithm>
#include <array>
#include <complex>
#include <fmt/format.h>
#include <gdal_priv.h>
#include <gsl/span>
#include <ogr_spatialref.h>
#include <optional>
#include <variant>

class OGRSimpleCurve;
class OGRFieldDefn;
class OGRPointIterator;
class OGRGeometryCollection;
class OGRMultiLineString;

namespace infra::gdal {

using namespace std::string_literals;

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

void registerGdal();
void unregisterGdal();

class Layer;

enum class MapType
{
    Memory,
    ArcAscii,
    GeoTiff,
    Gif,
    Png,
    Unknown
};

enum class VectorType
{
    Csv,
    Tab,
    ShapeFile,
    Xlsx,
    Unknown
};

Point<double> convertPointProjected(int32_t sourceEpsg, int32_t destEpsg, Point<double> point);
Point<double> projectedToGeoGraphic(int32_t epsg, Point<double>);
std::string projectionToFriendlyName(const std::string& projection);
std::string projectionFromEpsg(int32_t epsg);
int32_t projectionToGeoEpsg(const std::string& projection);
int32_t projectionToEpsg(const std::string& projection);
std::vector<const char*> createOptionsArray(const std::vector<std::string>& driverOptions);

MapType guessMapTypeFromFileName(const std::string& filePath);
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

using Geometry = std::variant<Point<double>, Line, MultiLine>;
using Field    = std::variant<int32_t, int64_t, double, std::string_view>;

class FieldDefinition
{
public:
    FieldDefinition(const char* name, const std::type_info& typeInfo);
    FieldDefinition(OGRFieldDefn* def);
    ~FieldDefinition();
    std::string_view name() const;
    const std::type_info& type() const;

    OGRFieldDefn* get() noexcept;

private:
    bool _hasOwnerShip;
    OGRFieldDefn* _def;
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
    Feature operator[](int64_t index) const;

    int fieldIndex(std::string_view name) const;
    void setSpatialFilter(Point<double> point);

    void createField(FieldDefinition& field);
    void createFeature(Feature& feature);

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

class DataSet
{
public:
    static DataSet create(const std::string& filePath);
    // if you know the type of the dataset, this will be faster as not all
    // drivers are queried
    static DataSet create(const std::string& filePath, VectorType type, const std::vector<std::string>& driverOptions = {});

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

    Layer getLayer(int index);
    Layer createLayer(const std::string& name, const std::vector<std::string>& driverOptions = {});

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

    GDALDataset* get() const
    {
        return _ptr;
    }

private:
    explicit DataSet(const std::string& filename);

    GDALDataset* _ptr = nullptr;
};

class Driver
{
public:
    static Driver create(MapType);
    static Driver create(VectorType);
    static Driver create(const std::string& filename);

    template <typename T>
    DataSet createDataSet(uint32_t rows, uint32_t cols, uint32_t numBands, const std::string& filename)
    {
        return DataSet(checkPointer(_driver->Create(filename.c_str(), cols, rows, numBands, TypeResolve<T>::value, nullptr), "Failed to create data set"));
    }

    template <typename T>
    DataSet createDataSetCopy(const DataSet& reference, const std::string& filename, const std::vector<std::string>& driverOptions = {})
    {
        auto options = createOptionsArray(driverOptions);
        return DataSet(checkPointer(_driver->CreateCopy(
                                        filename.c_str(),
                                        reference.get(),
                                        FALSE,
                                        options.size() == 1 ? nullptr : const_cast<char**>(options.data()),
                                        nullptr,
                                        nullptr),
            "Failed to create data set copy"));
    }

    MapType mapType() const;

private:
    explicit Driver(GDALDriver* driver)
    : _driver(driver)
    {
    }

    GDALDriver* _driver;
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
}
