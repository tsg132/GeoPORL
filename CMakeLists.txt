# # Python bindings CMakeLists.txt

# cmake_minimum_required(VERSION 3.18)
# project(GeoPORL CXX CUDA)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CUDA_STANDARD 17)

# # Check if CUDA is available
# include(CheckLanguage)
# check_language(CUDA)

# if(CMAKE_CUDA_COMPILER)
#     enable_language(CUDA)
#     message(STATUS "CUDA found: ${CMAKE_CUDA_COMPILER}")
#     set(CUDA_AVAILABLE TRUE)
    
#     # Set CUDA architecture (adjust for your GPU, Colab uses T4 = sm_75)
#     set(CMAKE_CUDA_ARCHITECTURES 75)
# else()
#     message(STATUS "CUDA not found, building CPU-only version")
#     set(CUDA_AVAILABLE FALSE)
# endif()

# include_directories(include)

# add_executable(run_example
#     src/run_example.cpp
#     src/env.cpp
#     src/replay.cpp
#     src/metrics.cpp
#     src/dynamics.cpp
#     src/shock.cpp
# )

cmake_minimum_required(VERSION 3.18)
project(GeoPORL LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Check for CUDA
include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    message(STATUS "CUDA found: ${CMAKE_CUDA_COMPILER}")
    set(CUDA_AVAILABLE TRUE)
    set(CMAKE_CUDA_ARCHITECTURES 75)
else()
    message(WARNING "CUDA compiler not found. Building CPU-only version.")
    set(CUDA_AVAILABLE FALSE)
endif()

# Include project headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# === Add CUDA include paths manually for IDEs ===
if(CUDA_AVAILABLE)
    # Try to locate CUDA root
    find_package(CUDAToolkit REQUIRED)
    include_directories(${CUDAToolkit_INCLUDE_DIRS})
endif()

# ------------------ Executables ------------------

# CPU-only executable
add_executable(run_example
    src/run_example.cpp
    src/env.cpp
    src/replay.cpp
    src/metrics.cpp
    src/dynamics.cpp
    src/shock.cpp
)

# CUDA RL trainer (optional)
if(CUDA_AVAILABLE)
    add_executable(train_pg
        src/rl/policy.cu
        src/env.cpp
        src/replay.cpp
        src/metrics.cpp
        src/dynamics.cpp
        src/shock.cpp
    )
    target_include_directories(train_pg PRIVATE include)
    target_link_libraries(train_pg PRIVATE ${CUDAToolkit_LIBRARIES} cublas)
endif()