all:
	g++ `GraphicsMagick++-config --cxxflags --cppflags` \
	    -O3 -I../ENIMDA \
	    -o example example.cpp ../ENIMDA/ENIMDA.cpp \
	    `GraphicsMagick++-config --ldflags --libs`
