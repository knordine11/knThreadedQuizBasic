QT += widgets multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fftstuff.cpp \
    fileloader.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    fftstuff.h \
    fftw3/fftw3.h \
    fileloader.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.txt \
    fftw3/COPYRIGHT \
    fftw3/fftw3.f \
    fftw3/fftw3.f03 \
    fftw3/fftw3l.f03 \
    fftw3/fftw3q.f03 \
    fftw3/libfftw3-3.dll \
    fftw3/libfftw3f-3.dll \
    fftw3/libfftw3l-3.dll \
    lessons.txt \
    v_sounds/click2.wav \
    v_sounds/click3.wav \
    v_sounds/click4.wav \
    v_sounds/click5.wav \
    v_sounds/click5L.wav \
    v_sounds/click6.wav \
    v_sounds/click6L.wav \
    v_sounds/v43.wav \
    v_sounds/v44.wav \
    v_sounds/v45.wav \
    v_sounds/v46.wav \
    v_sounds/v47.wav \
    v_sounds/v48.wav \
    v_sounds/v49.wav \
    v_sounds/v50.wav \
    v_sounds/v51.wav \
    v_sounds/v52.wav \
    v_sounds/v53.wav \
    v_sounds/v54.wav \
    v_sounds/v55.wav \
    v_sounds/v56.wav \
    v_sounds/v57.wav \
    v_sounds/v58.wav \
    v_sounds/v59.wav \
    v_sounds/v60.wav \
    v_sounds/v61.wav \
    v_sounds/v62.wav

INCLUDEPATH += $$PWD/fftw3
DEPENDPATH += $$PWD/fftw3

unix|win32: LIBS += -L$$PWD/fftw3/ -llibfftw3-3 -llibfftw3f-3 -llibfftw3l-3

RESOURCES += \
    Resources.qrc
