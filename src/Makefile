# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -g \
    -I./ \
    -I../dependencies/GLAD/include \
    -I../dependencies/GLFW/include \
    -I../dependencies/GLM

# Point at the GLFW MinGW libraries (adjust path relative to src)
GLFW_LIB_DIR := ../dependencies/GLFW/lib-mingw

# Linker flags: import-lib + system libs
LDFLAGS := -L$(GLFW_LIB_DIR) \
    -lglfw3dll \
    -lopengl32 \
    -lgdi32

# Sources (Adjust this to your rectangle-specific main file)
SRC := main_rectangles.cpp engine.cpp

# Add the source files for circle and rectangle drawing if you intend to use them later
# SRC += circle.cpp rectangle.cpp

OBJ := $(SRC:.cpp=.o) ../dependencies/GLAD/src/glad.o

# Target (Rename the executable for the rectangle project)
TARGET := ../bin/rectangles_app.exe

# Default rule (all)
all: $(TARGET)

# Create the output directory, then build the executable, and copy the dll.
$(TARGET): | ../bin $(OBJ) # Corrected dependency order.  Use | to create directory only once.
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@if exist ../builds/windows/glfw3.dll (copy /Y ../builds/windows/glfw3.dll ../bin) else (echo "Warning: glfw3.dll not found!")

../bin: # Rule to create the binary directory.
	@if not exist ../bin ( mkdir ../bin )

# Compile .cpp files to .o files.  This is a pattern rule.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile glad (adjust path relative to src)
../dependencies/GLAD/src/glad.o: ../dependencies/GLAD/src/glad.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(OBJ) $(TARGET) 2>nul || exit 0
