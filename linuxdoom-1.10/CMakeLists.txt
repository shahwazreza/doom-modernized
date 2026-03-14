cmake_minimum_required(VERSION 3.10)
project(doom C)

# Use C99 standard
set(CMAKE_C_STANDARD 99)

# Find SDL2
find_package(SDL2 REQUIRED)

# Gather all .c source files in this directory
file(GLOB SOURCES "*.c")

# Create the executable
add_executable(doom ${SOURCES})

# Tell the compiler where SDL2 headers are
target_include_directories(doom PRIVATE ${SDL2_INCLUDE_DIRS})

# Link against SDL2 and math library
target_link_libraries(doom PRIVATE ${SDL2_LIBRARIES} m X11 Xext)

# Some defines the Doom source expects
target_compile_definitions(doom PRIVATE
    LINUX
    NORMALUNIX
)
