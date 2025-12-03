# CMakeList.txt : CMake project for portals, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.26)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project ("portals")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SDL_SHARED "" OFF)
option(SDL_STATIC "" ON)
add_subdirectory(libs/SDL-release-3.2.26)
add_subdirectory(libs/geGL-master)
add_subdirectory(libs/glm-1.0.2)
add_subdirectory(libs/assimp-6.0.2)

add_executable (${PROJECT_NAME}
    "src/main.cpp"
    "src/gl_scene.cpp"
    "src/map_gen.cpp"
    "libs/cppgraphics/cppgraphics.cpp"
    "src/point.cpp"
    "src/room_shape.cpp"
    "src/draw.cpp"
    "src/model.cpp"
    "src/portal_visibility.cpp"
 )

find_package(OpenMP REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC libs/cppgraphics)

target_compile_definitions(${PROJECT_NAME} PUBLIC  SRC_DIR="${CMAKE_SOURCE_DIR}/src/")
target_compile_definitions(${PROJECT_NAME} PUBLIC  MODELS_DIR="${CMAKE_SOURCE_DIR}/models/")

target_link_libraries(${PROJECT_NAME} PUBLIC
    SDL3::SDL3
    geGL::geGL
    glm::glm
    assimp::assimp
    OpenMP::OpenMP_CXX
)
