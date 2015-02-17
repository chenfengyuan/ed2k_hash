TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp
macx{
    QMAKE_CXXFLAGS += -I/usr/local/Cellar/boost/1.56.0/include/ -I/usr/local/Cellar/openssl/1.0.1i/include/
    QMAKE_LIBS += -L/usr/local/Cellar/openssl/1.0.1i/lib/ -lcrypto
}
