TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lmysqlclient
LIBS += -lpthread
LIBS += -lhiredis
LIBS += -lssl
LIBS += -lcrypto
INCLUDEPATH +=./include/
INCLUDEPATH +=./src/
SOURCES += \
        redis/RedisConfig.cpp \
        redis/RedisTools.cpp \
        spider/WBspider.cpp \
        src/Mysql.cpp \
        src/TCPKernel.cpp \
        src/TCPNet.cpp \
        src/err_str.cpp \
        src/main.cpp \
        src/temp.cpp \
        src/thread_pool.cpp

HEADERS += \
    include/Mysql.h \
    include/TCPKernel.h \
    include/TCPNet.h \
    include/err_str.h \
    include/packdef.h \
    include/thread_pool.h \
    redis/RedisConfig.h \
    redis/RedisTools.h \
    spider/WBspider.h

DISTFILES += \
    src/Mysql.cpp.bak \
    src/makefile
