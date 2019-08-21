TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    FHT/TestFHT.cpp \
    FHT/Hendler.cpp \
    FHT/Task.cpp

HEADERS += \
    FHT/Hendler.h \
    FHT/iHendler.h \
    FHT/iTask.h \
    FHT/Task.h \
    FHT/Controller.h \
    FHT/iController.h
