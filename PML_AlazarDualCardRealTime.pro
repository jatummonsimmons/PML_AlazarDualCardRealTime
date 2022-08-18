QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11
CONFIG += console
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AlazarControlThread.cpp \
    dataProcessingThread.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    AlazarControlThread.h \
    acquisitionConfig.h \
    dataProcessingThread.h \
    mainwindow.h \
    qcustomplot.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32: LIBS += -L$$PWD/'../../../../../Program Files/AlazarTech/ATS-SDK/7.4.0/Samples_C/Library/x64/' -lATSApi

#INCLUDEPATH += $$PWD/'../../../../../Program Files/AlazarTech/ATS-SDK/7.4.0/Samples_C/Include'
#DEPENDPATH += $$PWD/'../../../../../Program Files/AlazarTech/ATS-SDK/7.4.0/Samples_C/Include'


#unix|win32: LIBS += -L$$PWD/../../../../../AlazarTech/Samples_C/Library/x64/ -lATSApi



unix|win32: LIBS += -L$$PWD/../../../../../AlazarTech/Samples_C/Library/x64/ -lATSApi

INCLUDEPATH += $$PWD/../../../../../AlazarTech/Samples_C/Include
DEPENDPATH += $$PWD/../../../../../AlazarTech/Samples_C/Include


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../../AlazarTech/Samples_C/Library/x64/ATSApi.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../../../../AlazarTech/Samples_C/Library/x64/libATSApi.a
