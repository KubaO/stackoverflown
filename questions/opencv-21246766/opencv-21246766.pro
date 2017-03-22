QT = widgets
CONFIG += c++11
TARGET = opencv-21246766
TEMPLATE = app
SOURCES = main.cpp
macx:LIBS += -L /opt/local/lib
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio
