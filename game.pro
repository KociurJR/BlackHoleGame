TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += /usr/local/include


INCLUDEPATH += /opt/homebrew/opt/sfml@2/include
LIBS += -L/opt/homebrew/opt/sfml@2/lib \
        -lsfml-graphics -lsfml-window -lsfml-system
