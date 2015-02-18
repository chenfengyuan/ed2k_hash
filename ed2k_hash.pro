TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp
QMAKE_CXXFLAGS += -Wall -Wextra
macx{
    QMAKE_CXXFLAGS += -I/usr/local/Cellar/boost/1.56.0/include/ -I/usr/local/Cellar/openssl/1.0.1i/include/
    QMAKE_LIBS += -L/usr/local/Cellar/openssl/1.0.1i/lib/ -lcrypto
}
linux{
    QMAKE_LIBS += -lcrypto
contains(QMAKE_HOST.arch, armv6l){
        linux-g++{
                QT -= core gui
                QMAKE_CXXFLAGS += -std=c++11
                QMAKE_LIBS += -lcrypto
        }
}
