#pragma once

#include "config.h"

#include "data/AsciiForecastWriter.h"
#include "data/Hdf5Buffer.h"
#include "data/RioObsProvider.h"
#include "data/SqlBuffer.h"
#include "data/XMLAQNetProvider.h"
#include "data/XmlGridProvider.h"
#include "data/TextGridProvider.h"
#include "data/IRCELMeteoProvider.h"
#include "data/StationInfoProvider.h"
#include "data/RioOutputBuffer.h"

#include "forecast/OVL.h"
#include "forecast/OVL_IRCEL_model1.h"
#include "forecast/OVL_IRCEL_model2.h"
#include "forecast/OVL_IRCEL_model3.h"

#include "mapping/InverseDistanceWeighting.h"

namespace opaq
{

// by invoking a method on the plugins we avoid the linker from not including
// the static libraries because noone seems to use them
inline std::vector<std::string> getPluginNames()
{
#ifdef STATIC_PLUGINS
    return {
        AsciiForecastWriter::name(),
        RioObsProvider::name(),
        SqlBuffer::name(),
        Hdf5Buffer::name(),
        XMLAQNetProvider::name(),
        XmlGridProvider::name(),
        TextGridProvider::name(),
        IRCELMeteoProvider::name(),
        StationInfoProvider::name(),
        RioOutputBuffer::name(),

        OVL::name(),
        OVL_IRCEL_model1::name(),
        OVL_IRCEL_model2::name(),
        OVL_IRCEL_model3::name(),

        InverseDistanceWeighting::name()
    };
#else
    return {};
#endif
}

}
