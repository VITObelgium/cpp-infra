#include "PluginFactory.h"

#include "data/AsciiForecastWriter.h"
#include "data/Hdf5Buffer.h"
#include "data/IRCELMeteoProvider.h"
#include "data/RioObsProvider.h"
#include "data/RioOutputBuffer.h"
#include "data/SqlBuffer.h"
#include "data/StationInfoProvider.h"
#include "data/TextGridProvider.h"
#include "data/XMLAQNetProvider.h"
#include "data/XmlGridProvider.h"

#include "forecast/OVL.h"
#include "forecast/OVL_IRCEL_model1.h"
#include "forecast/OVL_IRCEL_model2.h"
#include "forecast/OVL_IRCEL_model3.h"

namespace opaq {
std::unique_ptr<Component> PluginFactory::createPlugin(std::string_view name) const
{
    if (name == AsciiForecastWriter::name()) {
        return std::make_unique<AsciiForecastWriter>();
    } else if (name == Hdf5Buffer::name()) {
        return std::make_unique<Hdf5Buffer>();
    } else if (name == IRCELMeteoProvider::name()) {
        return std::make_unique<IRCELMeteoProvider>();
    } else if (name == RioObsProvider::name()) {
        return std::make_unique<RioObsProvider>();
    } else if (name == RioOutputBuffer::name()) {
        return std::make_unique<RioOutputBuffer>();
    } else if (name == SqlBuffer::name()) {
        return std::make_unique<SqlBuffer>();
    } else if (name == StationInfoProvider::name()) {
        return std::make_unique<StationInfoProvider>();
    } else if (name == TextGridProvider::name()) {
        return std::make_unique<TextGridProvider>();
    } else if (name == XMLAQNetProvider::name()) {
        return std::make_unique<XMLAQNetProvider>();
    } else if (name == XmlGridProvider::name()) {
        return std::make_unique<XmlGridProvider>();
    } else if (name == OVL::name()) {
        return std::make_unique<OVL>();
    } else if (name == OVL_IRCEL_model1::name()) {
        return std::make_unique<OVL_IRCEL_model1>();
    } else if (name == OVL_IRCEL_model2::name()) {
        return std::make_unique<OVL_IRCEL_model2>();
    } else if (name == OVL_IRCEL_model3::name()) {
        return std::make_unique<OVL_IRCEL_model3>();
    }

    throw InvalidArgumentsException("Unknown plugin name: {}", name);
}
}
