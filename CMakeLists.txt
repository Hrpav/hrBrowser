cmake_minimum_required(VERSION 3.21)

project(hrBrowser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(WEBKITGTK REQUIRED webkit2gtk-4.1)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(hrBrowser src/main.cpp)
target_sources(hrBrowser PRIVATE
  ui/ui.cpp
  ui/address_bar/address_bar.cpp
  ui/backward_button/backward_button.cpp
  ui/forward_button/forward_button.cpp
  ui/refresh_button/refresh_button.cpp
)

target_include_directories(hrBrowser PRIVATE
  ${WEBKITGTK_INCLUDE_DIRS}
  ${GTK3_INCLUDE_DIRS}
)

target_link_libraries(hrBrowser
  ${WEBKITGTK_LIBRARIES}
  ${GTK3_LIBRARIES}
)
