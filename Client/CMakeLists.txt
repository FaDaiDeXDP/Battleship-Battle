cmake_minimum_required(VERSION 3.0)
project(MySFMLProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)

# 添加SFML库
add_subdirectory(${CMAKE_SOURCE_DIR}/SFML)

# 包含SFML头文件
include_directories(${CMAKE_SOURCE_DIR}/SFML/include)

# 添加你的源文件
add_executable(MySFMLProject src/main.cpp)

# 链接SFML库
target_link_libraries(MySFMLProject sfml-graphics sfml-window sfml-system)
