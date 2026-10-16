add_executable(dtc_zone_test
    zone_test.cpp
    ../src/zone.cpp ../src/zone.h
    ../src/zonemanager.cpp ../src/zonemanager.h
)

target_include_directories(dtc_zone_test PRIVATE
    ../src
    ../../engine/src
    ../../engine/audio/src
    ../../plugins/interfaces
)

target_link_libraries(dtc_zone_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

add_executable(dtc_backend_test
    backend_test.cpp
    ../src/patchmanager.cpp ../src/patchmanager.h ../src/patchplan.cpp
    ../src/fixturelibrary.cpp ../src/fixturelibrary.h
    ../src/gdtfimporter.cpp ../src/gdtfimporter.h
    ../src/programmer.cpp ../src/programmer.h
    ../src/zone.cpp ../src/zone.h
    ../src/zonemanager.cpp ../src/zonemanager.h
    ../src/iomanager.cpp ../src/iomanager.h
    ../src/effecteditor.cpp ../src/effecteditor.h
    ../src/timelineeditor.cpp ../src/timelineeditor.h
    ../src/fxtools.cpp ../src/fxtools.h
    ../src/remoteserver.cpp ../src/remoteserver.h
    ../src/updatechecker.cpp ../src/updatechecker.h
    ../src/palettemanager.cpp ../src/palettemanager.h
    ../src/commandline.cpp ../src/commandline.h
    ../src/cuelisteditor.cpp ../src/cuelisteditor.h
    ../src/playbackmanager.cpp ../src/playbackmanager.h
    # the remote control page and its icon font
    ../remote/remote.qrc
    ../icons/icons.qrc
    ../../resources/fonts/fonts.qrc
    ../src/showsurface.cpp ../src/showsurface.h
    ../src/drivecontrol.cpp ../src/drivecontrol.h
    ../src/effectexchange.cpp ../src/effectexchange.h
    ../src/livemixer.cpp ../src/livemixer.h
    ../src/controlmapper.cpp ../src/controlmapper.h
    ../src/dtccore.cpp ../src/dtccore.h
    ../src/appsettings.cpp ../src/appsettings.h
)

target_include_directories(dtc_backend_test PRIVATE
    ../src
    ../../engine/src
    ../../engine/audio/src
    ../../plugins/interfaces
)

target_compile_definitions(dtc_backend_test PRIVATE
    DTC_GENERIC_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/Generic"
    DTC_RGB_SCRIPTS="${CMAKE_CURRENT_SOURCE_DIR}/../../resources/rgbscripts"
    DTC_AUDIO_PLUGINS="${CMAKE_BINARY_DIR}/engine/audio/plugins/sndfile"
    DTC_OSC_PLUGIN="${CMAKE_BINARY_DIR}/plugins/osc"
    DTC_ICONS="${CMAKE_CURRENT_SOURCE_DIR}/../icons"
)
# the MIDI plugin, when it is built (an APC connected here gets its LEDs ready)
if(TARGET midiplugin)
    target_compile_definitions(dtc_backend_test PRIVATE DTC_MIDI_PLUGIN="$<TARGET_FILE_DIR:midiplugin>")
endif()

target_link_libraries(dtc_backend_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::WebSockets
    Qt${QT_MAJOR_VERSION}::CorePrivate
    Qt${QT_MAJOR_VERSION}::GuiPrivate
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)
set_target_properties(dtc_backend_test PROPERTIES AUTORCC ON)
