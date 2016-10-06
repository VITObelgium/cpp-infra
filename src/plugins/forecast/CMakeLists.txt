add_library(mpl_feedforward
    MLP_FeedForwardModel.h
    MLP_FeedForwardModel.cpp
    Tune.h
    Tune.cpp
    scaler.h
    scaler.cpp
    layer.h
    layer.cpp
    feedforwardnet.h
    feedforwardnet.cpp
    transfcn.h
)
set_target_properties(mpl_feedforward PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(ovl_ircel_model1 MODULE
    OVL_IRCEL_model1.h
    OVL_IRCEL_model1.cpp
)

add_library(ovl_ircel_model2 MODULE
    OVL_IRCEL_model2.h
    OVL_IRCEL_model2.cpp
)

add_library(ovl_ircel_model3 MODULE
    OVL_IRCEL_model3.h
    OVL_IRCEL_model3.cpp
)

add_library(ovl MODULE
    OVL.h
    OVL.cpp
)

target_link_libraries(ovl_ircel_model1 mpl_feedforward opaqcore)
target_link_libraries(ovl_ircel_model2 mpl_feedforward opaqcore)
target_link_libraries(ovl_ircel_model3 mpl_feedforward opaqcore)
target_link_libraries(ovl mpl_feedforward opaqcore)
