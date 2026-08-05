cmake_minimum_required(VERSION 3.16)

project(Pulsar VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
    src/main.cpp
    src/input/InputManager.cpp
    src/input/InputManager.h
    src/emulator/EmulatorManager.cpp
    src/emulator/EmulatorManager.h
    src/emulator/CoreRegistry.cpp
    src/emulator/CoreRegistry.h
    src/emulator/ConsoleProfile.cpp
    src/emulator/ConsoleProfile.h
        src/emulator/LibretroBackend/LibretroCore.cpp
        src/emulator/LibretroBackend/LibretroCore.h
        src/emulator/LibretroBackend/Libretro.h
        src/emulator/LibretroBackend/LibretroEnvironment.h
        src/emulator/LibretroBackend/LibretroEnvironment.cpp
        src/emulator/LibretroBackend/LibretroVideo.cpp
        src/emulator/LibretroBackend/LibretroVideo.h
        src/emulator/LibretroBackend/LibretroInput.cpp
        src/emulator/LibretroBackend/LibretroInput.h
        src/emulator/LibretroBackend/LibretroAudio.cpp
        src/emulator/LibretroBackend/LibretroAudio.h
        src/emulator/LibretroBackend/LibretroTouch.cpp
        src/emulator/LibretroBackend/LibretroTouch.h
    src/audio/AudioManager.cpp
    src/audio/AudioManager.h
    src/library/Game.cpp
    src/library/Game.h
    src/library/LibraryManager.cpp
    src/library/LibraryManager.h
    src/ui/ConsoleWindow.cpp
    src/ui/ConsoleWindow.h
    src/ui/pages/SplashScreen.cpp
    src/ui/pages/SplashScreen.h
    src/ui/pages/BottomHomePage.cpp
    src/ui/pages/BottomHomePage.h
        src/ui/pages/TopHomePage.cpp
        src/ui/pages/TopHomePage.h
    src/ui/pages/EmulatorPage.cpp
    src/ui/screens/TopScreen.cpp
    src/ui/screens/TopScreen.h
    src/ui/screens/BottomScreen.cpp
    src/ui/screens/BottomScreen.h
    src/ui/widgets/GameTileWidget.cpp
    src/ui/widgets/GameTileWidget.h
    src/ui/widgets/BackgroundWidget.cpp
    src/ui/widgets/BackgroundWidget.h
    src/ui/widgets/BatteryWidget.cpp
    src/ui/widgets/BatteryWidget.h
    src/ui/widgets/EmulatorScreenWidget.cpp
    src/ui/widgets/EmulatorScreenWidget.h
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(Pulsar
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(Pulsar SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Pulsar
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(Pulsar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Pulsar PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

set_target_properties(Pulsar PROPERTIES
    WIN32_EXECUTABLE FALSE
)

include(GNUInstallDirs)
install(TARGETS Pulsar
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Pulsar)
endif()
