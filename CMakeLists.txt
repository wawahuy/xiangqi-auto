cmake_minimum_required(VERSION 3.14)
project(xiangqi_auto CXX)

set(APP_NAME "xiangqi_auto")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE HEADER_LIST
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE SRC_LIST
  "./src/*.cpp"
)

set(SRCS ./src/main.cpp)

# ImGui Library Target
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")
add_library(imgui STATIC
  "${IMGUI_DIR}/imgui.cpp"
  "${IMGUI_DIR}/imgui_demo.cpp"
  "${IMGUI_DIR}/imgui_draw.cpp"
  "${IMGUI_DIR}/imgui_tables.cpp"
  "${IMGUI_DIR}/imgui_widgets.cpp"
  "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
  "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)
target_include_directories(imgui PUBLIC
  "${IMGUI_DIR}"
  "${IMGUI_DIR}/backends"
)

add_executable(${APP_NAME} WIN32 ${SRCS} ${SRC_LIST} ${HEADER_LIST})

target_link_libraries(${APP_NAME} imgui ${DEPS}
  user32
  gdi32
  gdiplus
  comctl32
  comdlg32
  winhttp
  winmm
  windowsapp
  d3d11
  dxgi
)

target_include_directories(${APP_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_compile_definitions(${APP_NAME} PRIVATE
  UNICODE
  _UNICODE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
)

target_sources(${APP_NAME} PRIVATE
  ./src/xiangqi_auto.rc
)

source_group(
  TREE "${CMAKE_SOURCE_DIR}"
  FILES
    ${SRCS}
    ${SRC_LIST}
    ${HEADER_LIST}
    ./src/xiangqi_auto.rc
)