csrc = $(wildcard source/*.cpp)
obj = $(csrc:.cpp=.o)
EXECUTABLE = solvaminx
UNAME := $(shell uname)
CXXFLAGS = -std=c++17 -Wall #-std=c++0x
ifeq ($(UNAME), MINGW32_NT-6.1)
LDFLAGS = -lglfw3 -lopengl32 -lgdi32 -lglew32
else
LDFLAGS = -lGL -lGLU -lglfw -lGLEW -lX11 -lXxf86vm -lXrandr -lpthread -lXi
endif

build/$(EXECUTABLE): $(obj)
	@echo $(UNAME)
	mkdir -p build
	cp resources/dlls/* build/
	$(CXX) -o $@ $^ $(LDFLAGS)
#	cd build; ./$(EXECUTABLE).exe

%.d: %.c
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean launch
clean:
	rm -r build source/*.o

launch:
	cd build; ./$(EXECUTABLE).exe