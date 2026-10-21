/*
  DMX Total Control
  main.cpp

  Copyright (c) DMX Total Control contributors

  Based on Q Light Controller Plus
  Copyright (c) Heikki Junnila, Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QFile>
#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QTranslator>
#include <QSettings>
#include <QFileInfo>
#include <QLocale>
#include <QIcon>
#include <QUrl>
#include <QKeySequence>
#include <QQuickItem>
#include <qpa/qwindowsysteminterface.h>

#include "dtccore.h"
#include "zonemanager.h"
#include "patchmanager.h"
#include "programmer.h"
#include "iomanager.h"
#include "appsettings.h"
#include "fixturelibrary.h"
#include "effecteditor.h"
#include "timelineeditor.h"
#include "fxtools.h"
#include "remoteserver.h"
#include "showsurface.h"
#include "drivecontrol.h"
#include "livemixer.h"
#include "controlmapper.h"
#include "uiaudit.h"
#include "zone.h"
#include "qlcconfig.h"

#define DTC_QML_BACKEND "DMXTotalControl.Backend"

int main(int argc, char *argv[])
{
    // screenshots (--grab) leave the settings of the user alone: they use
    // their own, empty each time (the recent shows, the size, the guide...)
    bool grabbing = false;
    for (int i = 1; i < argc; i++)
        grabbing = grabbing || QByteArray(argv[i]).startsWith("--grab");

    // the size of the interface (Options), before the application exists
    const int uiScale = grabbing ? 100 : AppSettings::savedUiScale();
    if (uiScale != 100 && qEnvironmentVariableIsEmpty("QT_SCALE_FACTOR"))
        qputenv("QT_SCALE_FACTOR", QByteArray::number(uiScale / 100.0));

    // QApplication rather than QGuiApplication: some I/O plugins
    // still open QtWidgets configuration dialogs
    QApplication app(argc, argv);

    QApplication::setOrganizationName("DMXTotalControl");
    QApplication::setOrganizationDomain("dmxtotalcontrol.org");
    QApplication::setApplicationName(APPNAME);
    QApplication::setApplicationVersion(QString(APPVERSION));
    if (grabbing)
    {
        QApplication::setOrganizationName("DMXTotalControlGrab");
        QSettings().clear();
        QSettings().setValue("ui/guideShown", true);
    }

    // the icon of the windows and the taskbar (the D alone under 24 px)
    QIcon appIcon;
    for (int size : { 16, 20, 24, 32, 40, 48, 64, 128, 256 })
        appIcon.addFile(QStringLiteral(":/icons/dtc-%1.png").arg(size), QSize(size, size));
    QApplication::setWindowIcon(appIcon);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(APPNAME));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "Show file to open.", "[file]");
    QCommandLineOption localeOption(QStringList() << "l" << "locale",
                                    "UI language, e.g. es_ES or en_US.", "locale");
    parser.addOption(localeOption);
    QCommandLineOption grabOption("grab", "Developer: save a screenshot of the window and quit.", "file");
    parser.addOption(grabOption);
    QCommandLineOption auditOption("audit", "Developer: with --grab, write what is hard to read on that screen (contrast, shortened or overflowing texts) to this file.", "file");
    parser.addOption(auditOption);
    QCommandLineOption sceneOption("scene", "Developer: open the Scenes page on this scene ID.", "id");
    parser.addOption(sceneOption);
    QCommandLineOption editShowOption("edit-show", "Developer: open the Show page in edit mode, with this control selected.", "widget");
    parser.addOption(editShowOption);
    QCommandLineOption selectAllOption("select-all", "Developer: select every fixture in Program (features).");
    parser.addOption(selectAllOption);
    QCommandLineOption mixerOption("mixer", "Developer: open the Live page on the mixer.");
    parser.addOption(mixerOption);
    QCommandLineOption fxOption("fx", "Developer: open the Program page on the FX.");
    parser.addOption(fxOption);
    QCommandLineOption fxMacroOption("fx-macro", "Developer: put an FX macro on the selection.", "macro");
    parser.addOption(fxMacroOption);
    QCommandLineOption palettesOption("palettes", "Developer: open the Program page on the palettes.");
    parser.addOption(palettesOption);
    QCommandLineOption commandOption("command", "Developer: run a command line in Program (\"1 THRU 4 @ FULL\").", "command");
    parser.addOption(commandOption);
    QCommandLineOption keypadOption("keypad", "Developer: open the keypad of the command line.");
    parser.addOption(keypadOption);
    QCommandLineOption newShowOption("new-show", "Developer: start a new show (File > New) one second after opening.");
    parser.addOption(newShowOption);
    QCommandLineOption remoteOption("remote", "Developer: start the remote control on a port, without saving it.", "port");
    parser.addOption(remoteOption);
    QCommandLineOption learnOption("learn", "Developer: start in learn mode (mappings).");
    parser.addOption(learnOption);
    QCommandLineOption mappingOption("mapping", "Developer: open the Mappings page with this mapping ID selected.", "id");
    parser.addOption(mappingOption);
    QCommandLineOption patchViewOption("patch-view", "Developer: open the Patch page on a view (list, grid, plan, live) or a dialog (limits, matrix).", "view");
    parser.addOption(patchViewOption);
    QCommandLineOption playOption("play", "Developer: play this scene ID on start, outside of the groups (with --page 6: the DMX levels).", "id");
    parser.addOption(playOption);
    QCommandLineOption sizeOption("size", "Developer: the size of the window (1440x900), for screenshots.", "WxH");
    parser.addOption(sizeOption);
    QCommandLineOption loadCueOption("load-cue", "Developer: load a cue of a cuelist in the programmer (cuelist ID:cue index).", "id:index");
    parser.addOption(loadCueOption);
    QCommandLineOption goCueOption("go-cue", "Developer: play a cuelist from a cue, as its playback does (cuelist ID:cue index).", "id:index");
    parser.addOption(goCueOption);
    QCommandLineOption recordOption("record", "Developer: open the record dialog of the Program page.");
    parser.addOption(recordOption);
    QCommandLineOption cueSelectOption("cue-select", "Developer: select these cues (indexes: 3,4) in the cuelist open in Scenes (after --load-cue).", "indexes");
    parser.addOption(cueSelectOption);
    QCommandLineOption cueFadeOption("cue-fade", "Developer: one second after start, set the fade of a cue (cuelist ID:cue index:ms).", "id:index:ms");
    parser.addOption(cueFadeOption);
    QCommandLineOption renameOption("rename", "Developer: one second after start, rename a function (ID:name).", "id:name");
    parser.addOption(renameOption);
    QCommandLineOption laterOption("later", "Developer: run a command line 1.5 seconds after start (after --cue-fade and --rename).", "command");
    parser.addOption(laterOption);
    QCommandLineOption pbSlotsOption("pb-slots", "Developer: faders per playback page (8, 10, 12, 16, 20).", "count");
    parser.addOption(pbSlotsOption);
    QCommandLineOption pbSelectOption("pb-select", "Developer: select this playback (slot index from 0) of the current playback page.", "slot");
    parser.addOption(pbSelectOption);
    QCommandLineOption pbGoOption("pb-go", "Developer: one second after start, GO on these playbacks of the current page (slot indexes from 0: 0,1,1 = two GOs on the second one; p1 pauses the second one).", "slots");
    parser.addOption(pbGoOption);
    QCommandLineOption pbAssignOption("pb-assign", "Developer: the Playbacks page with the list of scenes to assign.");
    parser.addOption(pbAssignOption);
    QCommandLineOption pbRecordOption("pb-record", "Developer: arm Record on the Playbacks page (needs values in the programmer: --command).");
    parser.addOption(pbRecordOption);
    QCommandLineOption pbDialogOption("pb-dialog", "Developer: open a menu or dialog of the Playbacks page: menu, add, assign, properties (with :slot), pages, update.", "kind:slot");
    parser.addOption(pbDialogOption);
    QCommandLineOption pbNextOption("pb-next", "Developer: 1.7 seconds after start (after --pb-go), Next on a cue of a playback of the current page (slot:cue index).", "slot:cue");
    parser.addOption(pbNextOption);
    QCommandLineOption keysOption("keys", "Developer: from one second after start, a step every 100 ms, the focus item logged after each: page:N shows a page, key:Space presses a key sequence (hold:Space holds it, 3 auto-repeats), down:A and up:A press and release a key in two steps, text:abc types, click:x,y clicks (left button; click:x,y,shift or ,ctrl with that key held), drag:x1,y1,x2,y2 drags. Steps separated by ';'.", "steps");
    parser.addOption(keysOption);
    QCommandLineOption guideOption("guide", "Developer: open the first steps guide on this step (from 0).", "step");
    parser.addOption(guideOption);
    QCommandLineOption helpPageOption("help-page", "Developer: open the manual on this page (index, directo...).", "name");
    parser.addOption(helpPageOption);
    QCommandLineOption themeOption("theme", "Developer: use this theme (dark, light, contrast, night) for this run, without saving it.", "name");
    parser.addOption(themeOption);
    QCommandLineOption updateUrlOption("update-url", "Developer: check for updates at once against this release list (a GitHub API URL, or a file:/// JSON in its shape), without the repository rule.", "url");
    parser.addOption(updateUrlOption);
    QCommandLineOption pageOption("page", "Page to show at startup (0 Live, 1 Show, 2 Program, 3 Scenes, 4 Patch, 5 Fixtures, 6 Outputs, 7 Mappings, 8 Options, 9 Playbacks).", "index");
    parser.addOption(pageOption);
    parser.process(app);

    // The UI is written in English; other languages come from :/i18n
    QString language = parser.isSet(localeOption) ? parser.value(localeOption)
                                                   : QSettings().value("ui/language").toString();
    QLocale locale = language.isEmpty() ? QLocale() : QLocale(language);
    // dates and numbers in the language of the app, not the one of the system
    if (language.isEmpty() == false)
        QLocale::setDefault(locale);
    QTranslator translator;
    if (translator.load(locale, "dtc", "_", ":/i18n"))
        app.installTranslator(&translator);

    QQuickStyle::setStyle("Basic");

    DtcCore core;
    // the manual in the language of the interface (English without a translation)
    core.setUiLanguage(translator.isEmpty() ? QString("") : translator.language());
    if (parser.isSet(themeOption))
        core.settings()->setThemeOverride(parser.value(themeOption));

    qmlRegisterSingletonInstance(DTC_QML_BACKEND, 1, 0, "Core", &core);
    qmlRegisterUncreatableType<ZoneManager>(DTC_QML_BACKEND, 1, 0, "ZoneManager", "Owned by Core");
    qmlRegisterUncreatableType<Zone>(DTC_QML_BACKEND, 1, 0, "Zone", "Owned by ZoneManager");
    qmlRegisterUncreatableType<PatchManager>(DTC_QML_BACKEND, 1, 0, "PatchManager", "Owned by Core");
    qmlRegisterUncreatableType<Programmer>(DTC_QML_BACKEND, 1, 0, "Programmer", "Owned by Core");
    qmlRegisterUncreatableType<IOManager>(DTC_QML_BACKEND, 1, 0, "IOManager", "Owned by Core");
    qmlRegisterUncreatableType<AppSettings>(DTC_QML_BACKEND, 1, 0, "AppSettings", "Owned by Core");
    qmlRegisterUncreatableType<FixtureLibrary>(DTC_QML_BACKEND, 1, 0, "FixtureLibrary", "Owned by Core");
    qmlRegisterUncreatableType<EffectEditor>(DTC_QML_BACKEND, 1, 0, "EffectEditor", "Owned by Core");
    qmlRegisterUncreatableType<TimelineEditor>(DTC_QML_BACKEND, 1, 0, "TimelineEditor", "Owned by Core");
    qmlRegisterUncreatableType<FxMacros>(DTC_QML_BACKEND, 1, 0, "FxMacros", "Owned by Core");
    qmlRegisterUncreatableType<RemoteServer>(DTC_QML_BACKEND, 1, 0, "RemoteServer", "Owned by Core");
    qmlRegisterUncreatableType<UpdateChecker>(DTC_QML_BACKEND, 1, 0, "UpdateChecker", "Owned by Core");
    qmlRegisterUncreatableType<ShowSurface>(DTC_QML_BACKEND, 1, 0, "ShowSurface", "Owned by Core");
    qmlRegisterUncreatableType<DriveControl>(DTC_QML_BACKEND, 1, 0, "DriveControl", "Owned by Core");
    qmlRegisterUncreatableType<LiveMixer>(DTC_QML_BACKEND, 1, 0, "LiveMixer", "Owned by Core");
    qmlRegisterUncreatableType<ControlMapper>(DTC_QML_BACKEND, 1, 0, "ControlMapper", "Owned by Core");
    qmlRegisterUncreatableType<PaletteManager>(DTC_QML_BACKEND, 1, 0, "PaletteManager", "Owned by Core");
    qmlRegisterUncreatableType<CommandLine>(DTC_QML_BACKEND, 1, 0, "CommandLine", "Owned by Core");
    qmlRegisterUncreatableType<CuelistEditor>(DTC_QML_BACKEND, 1, 0, "CuelistEditor", "Owned by Core");
    qmlRegisterUncreatableType<PlaybackManager>(DTC_QML_BACKEND, 1, 0, "PlaybackManager", "Owned by Core");

    if (parser.positionalArguments().isEmpty() == false)
    {
        QFileInfo fi(parser.positionalArguments().first());
        core.openShow(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
    else
    {
        core.openStartupShow();
    }

    // keyboard mappings work on every page
    app.installEventFilter(core.mapping());

    QQmlApplicationEngine engine;
    // Qt.uiLanguage: the language of the interface ("en" without a translation)
    engine.setUiLanguage(translator.isEmpty() ? QStringLiteral("en") : translator.language());
    // Prefer the deployed QML modules: some Qt builds (e.g. MSYS2) also
    // register module qmldirs in resources, where their plugins can't be found
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("DMXTotalControl", "Main");

    QQuickWindow *window = engine.rootObjects().isEmpty() ? nullptr :
                           qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (window && parser.isSet(pageOption))
        window->setProperty("currentPage", parser.value(pageOption).toInt());
    if (window && parser.isSet(sceneOption))
        window->setProperty("startScene", parser.value(sceneOption).toInt());
    if (window && parser.isSet(editShowOption))
        window->setProperty("startEditShow", parser.value(editShowOption));
    if (window && parser.isSet(selectAllOption))
        core.programmer()->selectAll();
    if (window && parser.isSet(mixerOption))
        window->setProperty("showMixer", true);
    if (window && parser.isSet(fxOption))
        window->setProperty("showFx", true);
    if (window && parser.isSet(fxMacroOption))
        core.fxMacros()->apply(parser.value(fxMacroOption));
    if (window && parser.isSet(palettesOption))
        window->setProperty("showPalettes", true);
    if (window && parser.isSet(commandOption))
        core.commands()->execute(parser.value(commandOption));
    if (window && parser.isSet(keypadOption))
        window->setProperty("showKeypad", true);
    if (window && parser.isSet(newShowOption))
        QTimer::singleShot(1000, &core, [&core]() { core.newShow(); });
    // the remote control, when it is on
    core.remote()->apply();
    if (parser.isSet(remoteOption))
        core.remote()->startOn(parser.value(remoteOption).toInt());
    if (window && parser.isSet(learnOption))
        core.mapping()->setLearnMode(true);
    if (window && parser.isSet(mappingOption))
        window->setProperty("startMapping", parser.value(mappingOption).toInt());
    if (window && parser.isSet(patchViewOption))
        window->setProperty("startPatchView", parser.value(patchViewOption));
    if (window && parser.isSet(playOption))
    {
        core.effects()->togglePreview(parser.value(playOption).toUInt());
        window->setProperty("showLevels", true);
    }

    if (window && parser.isSet(sizeOption))
    {
        const QStringList size = parser.value(sizeOption).split('x');
        if (size.count() == 2)
            window->resize(size.at(0).toInt(), size.at(1).toInt());
    }
    if (window && parser.isSet(loadCueOption))
    {
        const QStringList cue = parser.value(loadCueOption).split(':');
        if (cue.count() == 2)
        {
            core.cuelists()->setSelectedId(cue.at(0).toInt());
            core.cuelists()->loadCue(cue.at(0).toUInt(), cue.at(1).toInt(), true);
        }
    }
    if (window && parser.isSet(cueSelectOption))
    {
        QVariantList indexes;
        for (const QString &index : parser.value(cueSelectOption).split(',', Qt::SkipEmptyParts))
            indexes.append(index.trimmed().toInt());
        window->setProperty("startCueSelection", indexes);
    }
    // edits a while after start, as the user would make them (selections kept?)
    if (window && parser.isSet(cueFadeOption))
    {
        const QStringList cue = parser.value(cueFadeOption).split(':');
        if (cue.count() == 3)
            QTimer::singleShot(1000, &core, [&core, cue]() {
                core.cuelists()->setCueTimes(cue.at(0).toUInt(), { cue.at(1).toInt() }, cue.at(2).toInt(), -1, -1);
            });
    }
    if (window && parser.isSet(renameOption))
    {
        const QString value = parser.value(renameOption);
        const int colon = int(value.indexOf(':'));
        if (colon > 0)
            QTimer::singleShot(1000, &core, [&core, value, colon]() {
                core.renameFunction(value.left(colon).toUInt(), value.mid(colon + 1));
            });
    }
    if (window && parser.isSet(laterOption))
    {
        const QString command = parser.value(laterOption);
        QTimer::singleShot(1500, &core, [&core, command]() { core.commands()->execute(command); });
    }
    if (window && parser.isSet(goCueOption))
    {
        const QStringList cue = parser.value(goCueOption).split(':');
        if (cue.count() == 2)
            core.playbacks()->goToFunction(cue.at(0).toInt(), cue.at(1).toInt());
    }
    if (window && parser.isSet(recordOption))
        window->setProperty("showRecord", true);
    if (window && parser.isSet(pbSlotsOption))
        core.playbacks()->setSlotsPerPage(parser.value(pbSlotsOption).toInt());
    if (window && parser.isSet(pbSelectOption))
        core.playbacks()->select(-1, parser.value(pbSelectOption).toInt());
    if (window && parser.isSet(pbAssignOption))
        window->setProperty("showPlaybackAssign", true);
    if (window && parser.isSet(pbRecordOption))
        window->setProperty("armPlaybackRecord", true);
    if (window && parser.isSet(guideOption))
        window->setProperty("startGuide", parser.value(guideOption).toInt());
    if (window && parser.isSet(helpPageOption))
        window->setProperty("startHelp", parser.value(helpPageOption));
    if (window && parser.isSet(pbDialogOption))
        window->setProperty("startPlaybackDialog", parser.value(pbDialogOption));
    if (window && parser.isSet(pbGoOption))
    {
        const QStringList slotList = parser.value(pbGoOption).split(',', Qt::SkipEmptyParts);
        QTimer::singleShot(1000, &core, [&core, slotList]() {
            for (const QString &slot : slotList)
                if (slot.trimmed().startsWith(QLatin1Char('p')) == false)
                    core.playbacks()->go(-1, slot.trimmed().toInt());
        });
        // "p2": pause the third one, once it plays
        QTimer::singleShot(1500, &core, [&core, slotList]() {
            for (const QString &slot : slotList)
                if (slot.trimmed().startsWith(QLatin1Char('p')))
                    core.playbacks()->pause(-1, slot.trimmed().mid(1).toInt());
        });
    }
    if (window && parser.isSet(pbNextOption))
    {
        const QStringList next = parser.value(pbNextOption).split(':');
        if (next.count() == 2)
            QTimer::singleShot(1700, &core, [&core, next]() {
                core.playbacks()->setNextCue(-1, next.at(0).toInt(), next.at(1).toInt());
            });
    }
    if (window && parser.isSet(keysOption))
    {
        // through the window system, as typed: shortcuts and focus as for real
        const QStringList steps = parser.value(keysOption).split(';', Qt::SkipEmptyParts);
        for (int i = 0; i < steps.count(); i++)
        {
            const QString step = steps.at(i).trimmed();
            QTimer::singleShot(1000 + i * 100, window, [window, step]() {
                using WSI = QWindowSystemInterface;
                const QString arg = step.section(QLatin1Char(':'), 1);
                if (step.startsWith(QLatin1String("page:")))
                {
                    window->setProperty("currentPage", arg.toInt());
                }
                else if (step.startsWith(QLatin1String("key:")) || step.startsWith(QLatin1String("hold:")))
                {
                    // hold: a key held down, with 3 auto-repeats
                    const QKeySequence sequence(arg);
                    if (sequence.isEmpty() == false)
                    {
                        const QKeyCombination combo = sequence[0];
                        const QString text = combo.key() == Qt::Key_Space ? QStringLiteral(" ") : QString();
                        const int repeats = step.startsWith(QLatin1String("hold:")) ? 3 : 0;
                        for (int r = 0; r <= repeats; r++)
                            WSI::handleKeyEvent<WSI::SynchronousDelivery>(window, QEvent::KeyPress, combo.key(), combo.keyboardModifiers(), text, r > 0);
                        WSI::handleKeyEvent<WSI::SynchronousDelivery>(window, QEvent::KeyRelease, combo.key(), combo.keyboardModifiers(), text);
                    }
                }
                else if (step.startsWith(QLatin1String("down:")) || step.startsWith(QLatin1String("up:")))
                {
                    // a key pressed in one step and released in a later one
                    // (with its text, as typed: "a" for A)
                    const QKeySequence sequence(arg);
                    if (sequence.isEmpty() == false)
                    {
                        const QKeyCombination combo = sequence[0];
                        const int key = combo.key();
                        const QString text = key == Qt::Key_Space ? QStringLiteral(" ")
                                           : key < 0x80 ? QString(QChar(key)).toLower() : QString();
                        WSI::handleKeyEvent<WSI::SynchronousDelivery>(window, step.startsWith(QLatin1String("down:")) ? QEvent::KeyPress : QEvent::KeyRelease,
                                                                      key, combo.keyboardModifiers(), text);
                    }
                }
                else if (step.startsWith(QLatin1String("text:")))
                {
                    for (const QChar c : arg)
                    {
                        const int key = c.toUpper().unicode();
                        WSI::handleKeyEvent<WSI::SynchronousDelivery>(window, QEvent::KeyPress, key, Qt::NoModifier, QString(c));
                        WSI::handleKeyEvent<WSI::SynchronousDelivery>(window, QEvent::KeyRelease, key, Qt::NoModifier, QString(c));
                    }
                }
                else if (step.startsWith(QLatin1String("click:")))
                {
                    const QPointF local(arg.section(QLatin1Char(','), 0, 0).toDouble(), arg.section(QLatin1Char(','), 1, 1).toDouble());
                    const QPointF global = window->mapToGlobal(local);
                    // click:x,y,shift / click:x,y,ctrl: with that key held
                    const QString held = arg.section(QLatin1Char(','), 2, 2);
                    const Qt::KeyboardModifiers mods = held == QLatin1String("shift") ? Qt::ShiftModifier
                                                     : held == QLatin1String("ctrl") ? Qt::ControlModifier : Qt::NoModifier;
                    WSI::handleMouseEvent<WSI::SynchronousDelivery>(window, local, global, Qt::LeftButton, Qt::LeftButton, QEvent::MouseButtonPress, mods);
                    WSI::handleMouseEvent<WSI::SynchronousDelivery>(window, local, global, Qt::NoButton, Qt::LeftButton, QEvent::MouseButtonRelease, mods);
                }
                else if (step.startsWith(QLatin1String("drag:")))
                {
                    // drag:x1,y1,x2,y2: pressed at the first point, moved in
                    // ten steps to the second, released there
                    const QStringList p = arg.split(QLatin1Char(','));
                    if (p.count() == 4)
                    {
                        const QPointF from(p.at(0).toDouble(), p.at(1).toDouble());
                        const QPointF to(p.at(2).toDouble(), p.at(3).toDouble());
                        WSI::handleMouseEvent<WSI::SynchronousDelivery>(window, from, window->mapToGlobal(from), Qt::LeftButton, Qt::LeftButton, QEvent::MouseButtonPress);
                        for (int i = 1; i <= 10; i++)
                        {
                            const QPointF at = from + (to - from) * (i / 10.0);
                            WSI::handleMouseEvent<WSI::SynchronousDelivery>(window, at, window->mapToGlobal(at), Qt::LeftButton, Qt::NoButton, QEvent::MouseMove);
                        }
                        WSI::handleMouseEvent<WSI::SynchronousDelivery>(window, to, window->mapToGlobal(to), Qt::NoButton, Qt::LeftButton, QEvent::MouseButtonRelease);
                    }
                }
                QQuickItem *focus = window->activeFocusItem();
                qInfo("keys: %s -> focus %s, playback keys %d", qPrintable(step),
                      focus != nullptr ? focus->metaObject()->className() : "none",
                      int(window->property("playbackKeys").toBool()));
            });
        }
    }

    if (window && parser.isSet(grabOption))
    {
        const QString grabFile = parser.value(grabOption);
        const QString auditFile = parser.value(auditOption);
        QTimer::singleShot(2000, window, [window, grabFile, auditFile]() {
            // what is hard to read on the screen grabbed, a line each
            if (auditFile.isEmpty() == false)
            {
                QFile file(auditFile);
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
                    file.write((UiAudit::run(window).join('\n') + '\n').toUtf8());
            }
            window->grabWindow().save(grabFile);
            QCoreApplication::quit();
        });
    }

    // updates: a check a day (not in the screenshots of the developer options),
    // or at once against another release list
    if (parser.isSet(updateUrlOption))
    {
        core.updates()->setApiUrl(parser.value(updateUrlOption));
        core.updates()->check(false);
    }
    else if (parser.isSet(grabOption) == false)
    {
        core.updates()->checkOnStartup();
        // and new fixtures in the Open Fixture Library, if it is downloaded
        core.library()->checkOnStartup(core.updates()->autoCheck());
    }

    const int result = app.exec();
    // the update the user left for the exit
    core.updates()->runPendingInstall();
    return result;
}
