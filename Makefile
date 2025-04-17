# Compiler
CXX      := g++
CXXFLAGS := -std=c++17 -g \
	-I./src \
	-I./dependencies/GLAD/include \
	-I./dependencies/GLFW/include \
	-I./dependencies/GLM

# Point at the GLFW MinGW libraries
GLFW_LIB_DIR := ./dependencies/GLFW/lib-mingw

# Linker flags: import‐lib + system libs
LDFLAGS := -L$(GLFW_LIB_DIR) \
	-lglfw3dll \
	-lopengl32 \
	-lgdi32

# Sources
SRC    := src/main.cpp src/engine.cpp
OBJ    := $(SRC:.cpp=.o) \
	dependencies/GLAD/src/glad.o

# Target
TARGET := bin/app.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	if not exist bin ( mkdir bin )
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)
	copy /Y builds\windows\glfw3.dll bin\

# Compile .cpp
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile glad
dependencies/GLAD/src/glad.o: dependencies/GLAD/src/glad.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OBJ) $(TARGET) 2>nul || exit 0
