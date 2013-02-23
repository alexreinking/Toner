#-------------------------------------------------
#
# Project created by QtCreator 2012-01-04T21:31:50
#
#-------------------------------------------------


TARGET = Toner

QT += gui widgets

!win32 {
    QT       += mobility multimediakit
    CONFIG   += mobility
    DEFINES += NDEBUG
    MOBILITY = multimedia
    INCLUDEPATH += /usr/include/QtMobility
    INCLUDEPATH += /usr/include/QtMultimediaKit
}
win32 {
    QT       += multimedia
}

TEMPLATE = app

SOURCES += \
    main.cpp \
    maindialog.cpp \
    overtoneanalyzer.cpp \
    utils.cpp \
    datareader.cpp \
    staticanalysisdialog.cpp

HEADERS += \
    ffft/OscSinCos.hpp \
    ffft/OscSinCos.h \
    ffft/FFTRealUseTrigo.hpp \
    ffft/FFTRealUseTrigo.h \
    ffft/FFTRealSelect.hpp \
    ffft/FFTRealSelect.h \
    ffft/FFTRealPassInverse.hpp \
    ffft/FFTRealPassInverse.h \
    ffft/FFTRealPassDirect.hpp \
    ffft/FFTRealPassDirect.h \
    ffft/FFTRealFixLenParam.h \
    ffft/FFTRealFixLen.hpp \
    ffft/FFTRealFixLen.h \
    ffft/FFTReal.hpp \
    ffft/FFTReal.h \
    ffft/DynArray.hpp \
    ffft/DynArray.h \
    ffft/def.h \
    ffft/Array.hpp \
    ffft/Array.h \
    maindialog.h \
    overtoneanalyzer.h \
    utils.h \
    datareader.h \
    staticanalysisdialog.h

FORMS += maindialog.ui \
    staticanalysisdialog.ui
