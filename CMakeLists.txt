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


# User-configurable library paths
set(OPENCV_LIB "" CACHE PATH "Path to OpenCV build directory")
set(ONNXRUNTIME_LIB "" CACHE PATH "Path to ONNX Runtime directory")

if(NOT OPENCV_LIB)
  message(FATAL_ERROR "OPENCV_LIB is not set. Please configure OPENCV_LIB pointing to OpenCV build folder (e.g. -DOPENCV_LIB=D:/thirdparty/opencv/build)")
endif()

if(NOT ONNXRUNTIME_LIB)
  message(FATAL_ERROR "ONNXRUNTIME_LIB is not set. Please configure ONNXRUNTIME_LIB pointing to ONNX Runtime folder (e.g. -DONNXRUNTIME_LIB=D:/thirdparty/onnxruntime-win-x64-1.27.0)")
endif()

# Configure OpenCV
if(EXISTS "${OPENCV_LIB}")
  set(OpenCV_DIR "${OPENCV_LIB}")
  set(OpenCV_ARCH "x64")
  set(OpenCV_RUNTIME "vc16")
  find_package(OpenCV REQUIRED)
  message(STATUS "OpenCV found: ${OpenCV_VERSION}")
else()
  message(FATAL_ERROR "OpenCV directory not found at: ${OPENCV_LIB}")
endif()

# Configure ONNX Runtime
if(EXISTS "${ONNXRUNTIME_LIB}")
  set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_LIB}/include")
  set(ONNXRUNTIME_LIBRARY "${ONNXRUNTIME_LIB}/lib/onnxruntime.lib")
  if(NOT EXISTS "${ONNXRUNTIME_LIBRARY}")
    message(FATAL_ERROR "onnxruntime.lib not found in: ${ONNXRUNTIME_LIB}/lib")
  endif()
  
  add_library(onnxruntime SHARED IMPORTED)
  set_target_properties(onnxruntime PROPERTIES
    IMPORTED_IMPLIB "${ONNXRUNTIME_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    IMPORTED_LOCATION "${ONNXRUNTIME_LIB}/lib/onnxruntime.dll"
  )
else()
  message(FATAL_ERROR "ONNX Runtime directory not found at: ${ONNXRUNTIME_LIB}")
endif()

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

add_executable(${APP_NAME} WIN32 ${SRC_LIST} ${HEADER_LIST})

target_link_libraries(${APP_NAME} imgui
  ${OpenCV_LIBS}
  onnxruntime
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

target_include_directories(${APP_NAME} PRIVATE 
  "${CMAKE_SOURCE_DIR}/src"
  ${OpenCV_INCLUDE_DIRS}
)

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
    ${SRC_LIST}
    ${HEADER_LIST}
    ./src/xiangqi_auto.rc
)

# Post-build copy DLLs to the target executable directory
add_custom_command(TARGET ${APP_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:opencv_world>
    $<TARGET_FILE_DIR:${APP_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:onnxruntime>
    $<TARGET_FILE_DIR:${APP_NAME}>
  COMMENT "Copying DLL dependencies to target directory"
)