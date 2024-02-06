cmake_minimum_required(VERSION 3.16)

# Must change the following variables.
set(APPLICATION_NAME "Mouse Extender")
set(APPLICATION_VERSION 1.0.0)
set(ORGANIZATION_NAME "Live2D Inc.")
set(ORGANIZATION_DOMAIN "live2d.com")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
                ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

project(mouse-extender VERSION ${APPLICATION_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32 AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc)
    set(ICON_RESOURCE resources/icon.rc)
endif()
if(APPLE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(MACOSX_ICON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns)
    set_source_files_properties(${MACOSX_ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Core WebSockets)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(${PROJECT_NAME}
    MACOSX_BUNDLE ${MACOSX_ICON_PATH}
    main.cpp
    version.h
    ${ICON_RESOURCE}
)

qt_add_qml_module(${PROJECT_NAME}
    URI NLPluginApi
    VERSION 1.0
    SOURCES
        client.h
        message.h
        text_loader.h
    QML_FILES
        qml/Main.qml
        qml/License.qml
)

qt_add_resources(${PROJECT_NAME} "resources"
    FILES
        resources/icon.png
        LICENSE
        licenses/qt.txt
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${PROJECT_NAME} PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::Core Qt6::WebSockets
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
