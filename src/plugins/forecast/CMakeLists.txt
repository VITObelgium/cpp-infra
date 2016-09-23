ADD_LIBRARY(mpl_feedforward
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
SET_TARGET_PROPERTIES(mpl_feedforward PROPERTIES POSITION_INDEPENDENT_CODE ON)

ADD_LIBRARY(ovl_ircel_model1 MODULE
    OVL_IRCEL_model1.h
    OVL_IRCEL_model1.cpp
)

ADD_LIBRARY(ovl_ircel_model2 MODULE
    OVL_IRCEL_model2.h
    OVL_IRCEL_model2.cpp
)

ADD_LIBRARY(ovl_ircel_model3 MODULE
    OVL_IRCEL_model3.h
    OVL_IRCEL_model3.cpp
)

ADD_LIBRARY(ovl MODULE
    OVL.h
    OVL.cpp
)

TARGET_LINK_LIBRARIES(ovl_ircel_model1 mpl_feedforward opaqcore)
TARGET_LINK_LIBRARIES(ovl_ircel_model2 mpl_feedforward opaqcore)
TARGET_LINK_LIBRARIES(ovl_ircel_model3 mpl_feedforward opaqcore)
TARGET_LINK_LIBRARIES(ovl mpl_feedforward opaqcore)
