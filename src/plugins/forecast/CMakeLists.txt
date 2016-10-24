add_library(mpl_feedforward
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

add_library(ovl_ircel_model1 ${PLUGIN_TYPE}
    OVL_IRCEL_model1.h
    OVL_IRCEL_model1.cpp
)

add_library(ovl_ircel_model2 ${PLUGIN_TYPE}
    OVL_IRCEL_model2.h
    OVL_IRCEL_model2.cpp
)

add_library(ovl_ircel_model3 ${PLUGIN_TYPE}
    OVL_IRCEL_model3.h
    OVL_IRCEL_model3.cpp
)

add_library(ovl ${PLUGIN_TYPE}
    OVL.h
    OVL.cpp
)

target_link_libraries(ovl_ircel_model1 mpl_feedforward opaqcore)
target_link_libraries(ovl_ircel_model2 mpl_feedforward opaqcore)
target_link_libraries(ovl_ircel_model3 mpl_feedforward opaqcore)
target_link_libraries(ovl mpl_feedforward opaqcore)

set (PLUGINS ${PLUGINS} ovl ovl_ircel_model1 ovl_ircel_model2 ovl_ircel_model3 PARENT_SCOPE)
