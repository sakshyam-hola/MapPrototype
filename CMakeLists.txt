cmake_minimum_required(VERSION 3.16)
project(MapPrototype VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Xml)

qt_add_executable(MapPrototype
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mapwidget.cpp
    mapwidget.h
    graph.cpp
    graph.h
    dijkstra.cpp
    dijkstra.h
    osmparser.cpp
    osmparser.h
    resources.qrc
)

target_link_libraries(MapPrototype PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
)

set_target_properties(MapPrototype PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MapPrototype)
endif()