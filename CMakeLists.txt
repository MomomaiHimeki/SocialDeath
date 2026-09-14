cmake_minimum_required(VERSION 3.20)
project(SocialDeath LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SocialDeath WIN32 src/main.cpp)
target_link_libraries(SocialDeath PRIVATE gdiplus user32 gdi32)

