# #####################################################################
# Automatically generated by qmake (2.01a) ?? 1? 31 14:36:12 2013
# #####################################################################
TEMPLATE = app
TARGET = watchDiamond

DEPENDPATH += .
CONFIG += qt thread dll
INCLUDEPATH += . \
    /usr/local/include/opencv /usr/local/hvux_camera/include \
    /usr/include/emc2

DEFINES += ULAPI

LIBS += -L/usr/local/lib -lcv -lhighgui -lml -lcxcore -lm -ldhhvux -ldhimghelp \
    -L/usr/lib -lemc -lnml -lemcini -lemchal

# Input
HEADERS += markWidget.h \
    camera.hh \
    photoCapture.hh \
    displayModel.h  \
    imageMeasure.hh \
    patternEditerWidget.h \
    markView.h \
    markEMC.h \
    loadRecordDialog.h \
    watchDetect.h \
    watchConfig.h \
    diamodWidget.h \
    emcsecnml.hh \
    emcsec.hh \
    iniwrite.hh

FORMS += markWidget.ui \
    loadRecordDialog.ui

SOURCES += markWidget.cpp \
    dhCamera.cc \
    photoCapture.cc \
    displayModel.cpp \
    imageMeasure.cc \
    iniwrite.cc \
    markView.cpp \
    markEMC.cpp \
    emcsecnml.cc \
    emcsec.cc \
    loadRecordDialog.cpp \
    watchDetect.cpp \
    main.cpp \
    watchConfig.cc \
    diamodWidget.cpp

RESOURCES +=

OTHER_FILES += \
    update_package/share/o_nc/scan.ngc \
    update_package/share/o_nc/pickup.ngc \
    update_package/bin/M106 \
    update_package/bin/M102 \
    update_package/configs/watchDiamond.hal \
    update_package/update.sh \
    create_update.sh \
    update_package/share/o_nc/watch.ngc \
    update_package/share/o_nc/setdiamond.ngc \
    update_package/share/o_nc/scanholes.ngc \
    update_package/update_vm.sh
