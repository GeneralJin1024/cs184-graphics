include pngwriter/make.include

CLASSES=Vector.h Material.h Light.h GeoObject.h Camera.h
CXX=g++
CXXFLAGS= -O3 -Wall -Wno-deprecated -std=c++11 -DNO_FREETYPE $(FT_ARG_CFLAGS)
INC=  -Ipngwriter/src/ -I$(PREFIX)/include/
LIBS= -Lpngwriter/src -L$(PREFIX)/lib/ -lz -lpngwriter -lpng $(FT_ARG_LIBS)

all: $(CLASSES) $(RAYTRACER)
	$(CXX) $(CXXFLAGS) $(INC) raytracer.cpp -o raytracer $(LIBS)
clean:
	rm $(BINARY)
