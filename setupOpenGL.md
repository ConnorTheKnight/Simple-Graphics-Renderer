# Parallel OpenGL Shape Renderer

This project implements a parallel shape renderer using OpenGL that visualizes squares and circles with culling based on z-depth. The application uses multithreading for both the culling and drawing algorithms, as derived from the SquareAlgorithm.cpp and CircleAlgorithmParallel.cpp source files.

## Prerequisites

1. **Docker Desktop** - [Install Docker Desktop](https://www.docker.com/products/docker-desktop/)
2. **VcXsrv X Server** - [Download VcXsrv](https://sourceforge.net/projects/vcxsrv/)

## Setup Instructions

### 1. Install VcXsrv X Server

Download and install VcXsrv from the link provided above. This will allow you to forward graphical applications from the Docker container to your Windows desktop.

### 2. Start the X Server

1. Launch XLaunch (part of VcXsrv)
2. Select "Multiple windows" and click Next
3. Set "Display number" to 0 and click Next
4. Select "Start no client" and click Next
5. **Important**: Check the "Disable access control" option
6. Click Finish to start the X Server

### 3. Clone this Repository

```bash
git clone [repository-url]
cd [repository-directory]
```

### 4. Run the Application

#### Option 1: Using the batch script (Windows)

Simply run the provided batch script:

```bash
run-app.bat
```

#### Option 2: Manual startup

Set the DISPLAY environment variable (replace with your IP if needed):

- On Windows (PowerShell):
  ```powershell
  $env:DISPLAY = "127.0.0.1:0.0"
  ```

- On Windows (Command Prompt):
  ```cmd
  SET DISPLAY=127.0.0.1:0.0
  ```

Then build and run the Docker container:

```bash
docker-compose build
docker-compose up
```

## Using the Application

Once running, you'll see a window displaying the rendered shapes.

### Controls:

- **'c'** - Switch to circle mode
- **'s'** - Switch to square mode
- **'r'** - Recalculate culling and grid
- **'+'** - Increase thread count
- **'-'** - Decrease thread count
- **'q' or ESC** - Quit the application

## Parallel Implementation Details

This application implements parallel processing in two key areas:

### 1. Culling Algorithm

- The culling algorithm determines if a shape is completely covered by another shape with higher Z-value.
- For circles, it uses the formula: `sqrt((XB-XA)^2 + (YB-YA)^2) < radiusB`
- For squares, it uses the containment check: `(XA>XB)&&(YA>YB)&&(ZA<ZB)&&(XA+lengthA<XB+lengthB)&&(YA+lengthA<YB+lengthB)`
- The shapes are divided among multiple threads for parallel processing
- Mutexes ensure thread-safe updates to the cull array

### 2. Drawing Algorithm

- The drawing algorithm fills the grid based on which grid units are covered by visible shapes
- For circles, it checks each point within a bounding square using the formula: `sqrt((deltaX*deltaX)+(deltaY*deltaY)) < radius`
- For squares, it simply fills all grid cells within the square bounds
- The shape processing is divided among threads for parallel execution
- Each shape can further distribute its grid units among multiple worker threads
- Mutexes protect the shared isFilled grid from race conditions

### 3. Thread Management

- The number of threads defaults to the hardware concurrency of your system
- You can adjust the thread count during runtime using '+' and '-' keys
- The work distribution adapts to the available threads for optimal performance
- For large shapes, additional sub-threads are spawned to process grid cells in parallel

## Providing Custom Input

By default, the application generates random shapes. To provide custom input:

```bash
docker-compose run --rm opengl-app ./shape_renderer -i
```

Then enter the grid dimensions, number of shapes, number of threads, and shape data according to the expected format:

```
[verticalExtentOfGrid] [horizontalExtentOfGrid]
[numberOfShapesToRender]
[numberThreads]
[radius/sideLength] [x] [y] [z]
[radius/sideLength] [x] [y] [z]
...
```

## Performance Considerations

- The application uses thread-local computations when possible to minimize lock contention
- The thread count can be adjusted to find the optimal number for your specific hardware
- Very large numbers of shapes or grid sizes might benefit from more sophisticated spatial partitioning algorithms

## File Structure

- `main.cpp` - The main application code with parallel implementation
- `Dockerfile` - Container configuration including pthread support
- `docker-compose.yml` - Container orchestration
- `run-app.bat` - Helper script for Windows users

## Troubleshooting

- **Error connecting to X server**: Make sure VcXsrv is running with "Disable access control" checked
- **Black window**: Verify your graphics drivers support OpenGL
- **DISPLAY variable issues**: If using WSL2, you may need to set DISPLAY to the WSL2 host IP
- **Performance issues**: Try adjusting the number of threads using the '+' and '-' keys