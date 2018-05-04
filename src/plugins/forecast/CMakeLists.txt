add_library(mpl_feedforward STATIC
    MLP_FeedForwardModel.h
    MLP_FeedForwardModel.cpp
    scaler.h
    scaler.cpp
    layer.h
    layer.cpp
    feedforwardnet.h
    feedforwardnet.cpp
    transfcn.h
)
set_target_properties(mpl_feedforward PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(mpl_feedforward PUBLIC Eigen3::Eigen TinyXml::TinyXml)

add_library(ovl_ircel_model1 STATIC
    OVL_IRCEL_model1.h
    OVL_IRCEL_model1.cpp
)

add_library(ovl_ircel_model2 STATIC
    OVL_IRCEL_model2.h
    OVL_IRCEL_model2.cpp
)

add_library(ovl_ircel_model3 STATIC
    OVL_IRCEL_model3.h
    OVL_IRCEL_model3.cpp
)

add_library(ovl STATIC
    OVL.h
    OVL.cpp
)

target_link_libraries(ovl_ircel_model1 mpl_feedforward opaqcore)
target_link_libraries(ovl_ircel_model2 mpl_feedforward opaqcore)
target_link_libraries(ovl_ircel_model3 mpl_feedforward opaqcore)
target_link_libraries(ovl mpl_feedforward opaqcore)

if (NOT STATIC_PLUGINS)
    set_target_properties(ovl_ircel_model1 PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(ovl_ircel_model2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(ovl_ircel_model3 PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(ovl PROPERTIES POSITION_INDEPENDENT_CODE ON)

    add_library(ovl_ircel_model1plugin    MODULE OVL_IRCEL_model1Plugin.cpp)
    add_library(ovl_ircel_model2plugin    MODULE OVL_IRCEL_model2Plugin.cpp)
    add_library(ovl_ircel_model3plugin    MODULE OVL_IRCEL_model3Plugin.cpp)
    add_library(ovlplugin                 MODULE OVLPlugin.cpp)

    target_link_libraries(ovl_ircel_model1plugin ovl_ircel_model1)
    target_link_libraries(ovl_ircel_model2plugin ovl_ircel_model2)
    target_link_libraries(ovl_ircel_model3plugin ovl_ircel_model3)
    target_link_libraries(ovlplugin ovl)
endif ()

set (PLUGINS ${PLUGINS} ovl ovl_ircel_model1 ovl_ircel_model2 ovl_ircel_model3 PARENT_SCOPE)
