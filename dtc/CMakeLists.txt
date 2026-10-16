set(module_name "${APPBINARY}")

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

set(SRC_FILES
    src/main.cpp
    src/appsettings.cpp src/appsettings.h
    src/dtccore.cpp src/dtccore.h
    src/effecteditor.cpp src/effecteditor.h
    src/timelineeditor.cpp src/timelineeditor.h
    src/fxtools.cpp src/fxtools.h
    src/remoteserver.cpp src/remoteserver.h
    src/updatechecker.cpp src/updatechecker.h
    src/palettemanager.cpp src/palettemanager.h
    src/commandline.cpp src/commandline.h
    src/cuelisteditor.cpp src/cuelisteditor.h
    src/playbackmanager.cpp src/playbackmanager.h
    src/showsurface.cpp src/showsurface.h
    src/drivecontrol.cpp src/drivecontrol.h
    src/effectexchange.cpp src/effectexchange.h
    src/livemixer.cpp src/livemixer.h
    src/controlmapper.cpp src/controlmapper.h
    src/fixturelibrary.cpp src/fixturelibrary.h
    src/gdtfimporter.cpp src/gdtfimporter.h
    src/iomanager.cpp src/iomanager.h
    src/patchmanager.cpp src/patchmanager.h src/patchplan.cpp
    src/programmer.cpp src/programmer.h
    src/zone.cpp src/zone.h
    src/zonemanager.cpp src/zonemanager.h
)

set(QML_FILES
    qml/Main.qml
    qml/Theme.qml
    qml/components/AppButton.qml
    qml/components/AppMenu.qml
    qml/components/AppSwitch.qml
    qml/components/HSlider.qml
    qml/components/IconLabel.qml
    qml/components/MapTarget.qml
    qml/components/NavButton.qml
    qml/components/Panel.qml
    qml/components/StagePlan.qml
    qml/components/TextPrompt.qml
    qml/components/SceneBrowser.qml
    qml/components/FxShape.qml
    qml/components/FxShapePicker.qml
    qml/components/FxControls.qml
    qml/components/ReleaseNotes.qml
    qml/components/UpdateBanner.qml
    qml/components/FxPreview.qml
    qml/components/LockDialog.qml
    qml/components/TimeSpin.qml
    qml/components/VFader.qml
    qml/live/LivePage.qml
    qml/live/ZoneColumn.qml
    qml/live/SceneTile.qml
    qml/live/FunctionPicker.qml
    qml/live/SceneProperties.qml
    qml/live/MixerStrip.qml
    qml/live/MixerRow.qml
    qml/show/ShowPage.qml
    qml/show/SectionNode.qml
    qml/show/SectionView.qml
    qml/show/SectionProperties.qml
    qml/show/SurfaceButton.qml
    qml/show/SurfaceFader.qml
    qml/show/SurfaceDial.qml
    qml/programmer/ProgrammerPage.qml
    qml/programmer/FixtureTile.qml
    qml/programmer/ChannelFader.qml
    qml/programmer/RecordDialog.qml
    qml/programmer/FeaturesView.qml
    qml/programmer/FxPanel.qml
    qml/programmer/CommandBar.qml
    qml/programmer/PalettesView.qml
    qml/programmer/XYGrid.qml
    qml/programmer/ColorField.qml
    qml/scenes/ScenesPage.qml
    qml/scenes/StaticEditor.qml
    qml/scenes/StepsEditor.qml
    qml/scenes/MoveFxEditor.qml
    qml/scenes/ColourFxEditor.qml
    qml/scenes/CurveFxEditor.qml
    qml/scenes/PlaybackPanel.qml
    qml/scenes/RackEditor.qml
    qml/scenes/SuperSceneEditor.qml
    qml/scenes/FxSceneEditor.qml
    qml/scenes/TimelineTrack.qml
    qml/scenes/TimelineBlock.qml
    qml/scenes/CuelistView.qml
    qml/scenes/TrackingSheet.qml
    qml/scenes/CuelistOptions.qml
    qml/scenes/CueExistsDialog.qml
    qml/playbacks/PlaybacksPage.qml
    qml/playbacks/PlaybackStrip.qml
    qml/playbacks/PlaybackSlotDialog.qml
    qml/playbacks/PlaybackAssignDialog.qml
    qml/playbacks/SelectedCuelist.qml
    qml/patch/PatchPage.qml
    qml/patch/AddFixtureDialog.qml
    qml/patch/EditFixtureDialog.qml
    qml/patch/ReaddressDialog.qml
    qml/patch/FixtureOptionsDialog.qml
    qml/patch/MatrixDialog.qml
    qml/patch/PatchGrid.qml
    qml/fixtures/FixtureEditorPage.qml
    qml/fixtures/CapabilitiesDialog.qml
    qml/fixtures/LibraryPicker.qml
    qml/setup/OutputsPage.qml
    qml/setup/DmxMonitor.qml
    qml/mapping/MappingsPage.qml
    qml/mapping/MappingEditor.qml
    qml/settings/OptionsPage.qml
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Fonts shared with QLC+: Roboto (Apache 2.0) and FontAwesome Free (OFL).
# The icon: icons/make-icons.sh makes its files from the SVGs
set(RESOURCE_FILES ../resources/fonts/fonts.qrc remote/remote.qrc icons/icons.qrc)

if(WIN32)
    # the icon of the executable and its version information
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" DTC_VERSION_MATCH "${APPVERSION}")
    set(DTC_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(DTC_VERSION_MINOR ${CMAKE_MATCH_2})
    set(DTC_VERSION_PATCH ${CMAKE_MATCH_3})
    set(DTC_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/icons/dtc.ico")
    configure_file(icons/dtc.rc.in ${CMAKE_CURRENT_BINARY_DIR}/dtc.rc @ONLY)
    list(APPEND RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/dtc.rc)
    qt_add_executable(${module_name} WIN32 ${SRC_FILES} ${RESOURCE_FILES})
else()
    qt_add_executable(${module_name} ${SRC_FILES} ${RESOURCE_FILES})
endif()

qt_add_qml_module(${module_name}
    URI DMXTotalControl
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

# Translations, embedded under :/i18n. Update the .ts files with the
# ${module_name}_lupdate target
qt_add_translations(${module_name}
    TS_FILES i18n/dtc_es_ES.ts
    RESOURCE_PREFIX /i18n
    LUPDATE_OPTIONS -no-obsolete -locations none
)

target_include_directories(${module_name} PRIVATE
    src
    ../engine/src
    ../engine/audio/src
    ../plugins/interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::QuickControls2
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::WebSockets
    Qt${QT_MAJOR_VERSION}::CorePrivate
    Qt${QT_MAJOR_VERSION}::GuiPrivate
    qlcplusengine
)

# The repository whose GitHub releases the updates come from
set(DTC_GITHUB_REPO "joseramos1999/DMXTotalControl" CACHE STRING
    "GitHub repository (owner/name) of the releases the updates come from")
target_compile_definitions(${module_name} PRIVATE DTC_UPDATE_REPO="${DTC_GITHUB_REPO}")

install(TARGETS ${module_name} DESTINATION ${INSTALLROOT}/${BINDIR})

# Basic generic fixtures, see fixtures/generate-generic.pl
install(DIRECTORY fixtures/Generic/ DESTINATION ${INSTALLROOT}/${FIXTUREDIR}/DMXTotalControl
        FILES_MATCHING PATTERN "*.qxf")

# The license and the notices go with the program (Apache License 2.0,
# section 4), and the license of the icon font
if(WIN32)
    install(FILES ${CMAKE_SOURCE_DIR}/COPYING DESTINATION ${INSTALLROOT}/${DATADIR} RENAME LICENSE.txt)
    install(FILES ${CMAKE_SOURCE_DIR}/NOTICE DESTINATION ${INSTALLROOT}/${DATADIR} RENAME NOTICE.txt)
    install(FILES ${CMAKE_SOURCE_DIR}/resources/fonts/OFL.txt DESTINATION ${INSTALLROOT}/${DATADIR} RENAME FontAwesome-OFL.txt)
endif()

add_subdirectory(test)
