######################################################################
# Automatically generated by qmake (2.01a) Sat Nov 7 01:19:33 2009
######################################################################

TEMPLATE = app
TARGET = PhotoTheater
INCLUDEPATH += ../Libs/FCam/include

QT += opengl network

CONFIG += debug warn_on

HEADERS += OverlayWidget.h CameraThread.h \
    MainWindow.h \
    GLWidget.h \
    ScrollArea.h \
    ImageItem.h \
    UserDefaults.h \
    CutWidget.h \
    CutLabel.h \
    CutOut.h \
    SpaceView.h \
    GLCommon.h \
    ViewFinder.h \
    CameraParameters.h \
    AdjustmentWidget.h \
    SettingsTree.h \
    VisualizationWidget.h \
    ImportExport.h \
    ThumbnailView.h \
    quazip/zip.h \
    quazip/unzip.h \
    quazip/quazipnewinfo.h \
    quazip/quazipfileinfo.h \
    quazip/quazipfile.h \
    quazip/quazip.h \
    quazip/quacrc32.h \
    quazip/quachecksum32.h \
    quazip/quaadler32.h \
    quazip/JlCompress.h \
    quazip/ioapi.h \
    quazip/crypt.h \
    Utilies.h \
    SceneSelector.h \
    HttpHandler.h \
    Scene.h
SOURCES += PhotoTheater.cpp OverlayWidget.cpp CameraThread.cpp \
    MainWindow.cpp \
    GLWidget.cpp \
    ScrollArea.cpp \
    ImageItem.cpp \
    UserDefaults.cpp \
    CutWidget.cpp \
    CutLabel.cpp \
    CutOut.cpp \
    SpaceView.cpp \
    ViewFinder.cpp \
    CameraParameters.cpp \
    AdjustmentWidget.cpp \
    SettingsTree.cpp \
    VisualizationWidget.cpp \
    ImportExport.cpp \
    ThumbnailView.cpp \
    quazip/zip.c \
    quazip/unzip.c \
    quazip/quazipnewinfo.cpp \
    quazip/quazipfile.cpp \
    quazip/quazip.cpp \
    quazip/quacrc32.cpp \
    quazip/quaadler32.cpp \
    quazip/JlCompress.cpp \
    quazip/ioapi.c \
    Utilies.cpp \
    SceneSelector.cpp \
    HttpHandler.cpp \
    Scene.cpp

CONFIG += qt 
LIBS += -L../Libs/FCam/ -lpthread -ljpeg -lFCam
