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

TARGET_LINK_LIBRARIES(ovl_ircel_model1 opaqcore mpl_feedforward)
TARGET_LINK_LIBRARIES(ovl_ircel_model2 opaqcore mpl_feedforward)
TARGET_LINK_LIBRARIES(ovl_ircel_model3 opaqcore mpl_feedforward)
