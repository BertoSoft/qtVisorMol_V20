cmake_minimum_required(VERSION 3.19)
project(VisorMol_V10 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# Habilitamos explícitamente el procesador automático de recursos de Qt (.qrc)
set(CMAKE_AUTORCC ON)

qt_add_executable(VisorMol_V10
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    recursos.qrc
    lienzomolecula.h
    lienzomolecula.cpp
    optimizaciongeometrica.h optimizaciongeometrica.cpp optimizaciongeometrica.ui
)

target_link_libraries(VisorMol_V10
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS VisorMol_V10
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET VisorMol_V10
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
