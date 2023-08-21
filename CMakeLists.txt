cmake_minimum_required(VERSION 3.5)

project(RoadscopePcbInspector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Charts Gui SeiralPort Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Charts Gui SerialPort Widgets LinguistTools)

qt_standard_project_setup()

set(TS_FILES RoadscopePcbInspector_ko_KR.ts)

set(PROJECT_SOURCES
        console.cpp console.h
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        mainwindowcontents.cpp mainwindowcontents.h mainwindowcontents.ui
        customstringlistmodel.h
        contentwidget.cpp contentwidget.h
        horizontalbarwidget.cpp horizontalbarwidget.h
        statitem.h statitem.cpp
        settingsdialog.cpp settingsdialog.h settingsdialog.ui
        ${TS_FILES}
)



if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RoadscopePcbInspector
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
        customstringlistmodel.cpp
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

else()
    add_executable(RoadscopePcbInspector
        ${PROJECT_SOURCES}
    )

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()



target_link_libraries(RoadscopePcbInspector
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Charts
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::SerialPort
        Qt${QT_VERSION_MAJOR}::Widgets
)


set_target_properties(RoadscopePcbInspector PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)


# Resources:
#set(RoadscopePcbInspector_resource_files
#    "images/application-exit.png"
#    "images/clear.png"
#    "images/connect.png"
#    "images/disconnect.png"
#    "images/settings.png"
#    "images/aim_inspector_icon.png"
#)


#qt_add_resources(RoadscopePcbInspector "images"
#    PREFIX
#        "/"
#    FILES
#        ${RoadscopePcbInspector_resource_files}
#)


include(GNUInstallDirs)
install(TARGETS RoadscopePcbInspector
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RoadscopePcbInspector)
endif()
