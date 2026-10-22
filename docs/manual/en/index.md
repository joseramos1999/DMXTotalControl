# DMX Total Control manual

DMX Total Control is a free program to control DMX lighting from a Windows computer. It is made for live work: you play scenes by groups, program fast with the programmer, run cuelists with GO on the playbacks and, if you like, build your own touch screen (the SHOW) and use it from your phone too.

With DMX Total Control you can:

- Patch the fixtures of your rig with a library of thousands of definitions, plus the Open Fixture Library, GDTF import and your own fixtures.
- Give dimmer, colour, position, gobos and any other channel to the selected fixtures, with palettes, FX and a command line.
- Record static scenes, steps, FX, super scenes with a timeline and audio, and cuelists with tracking.
- Play them in Live by groups, on the Playbacks with GO or from your SHOW.
- Send DMX out through DMX USB interfaces, Art-Net or sACN, and control everything from MIDI, OSC, DMX input or the keyboard.

Shows are saved as `.qxw` files, the same format QLC+ uses, so you can open shows made with QLC+. If the program is already open and you open a show from the Explorer (double-click on the file), it opens in that same window, which comes to the front (it asks about the unsaved changes first).

## How to open this manual

Press **F12** or the **“?”** button in the top bar. The manual opens on the right side of the window, on the page that explains the page you are on: if you are in Program, it opens on [Program](programar.md). Links take you to other pages of the manual; from the panel you can go back to the previous page or to this “Contents” page. You can also open it from Options › Help › **“Open the manual”**.

## The guide and the example shows

- **The first steps guide**: a card at the bottom right that takes you through the pages in five steps (the fixtures, a look, a scene, Live and the playbacks). It appears the first time you open the program; after that, in Options › Help › **“Start the guide”**. You can use the pages while it is open. See [First steps](primeros-pasos.md).
- **The example shows**: ready-made shows to open and try. They are in the **▾** arrow next to “Open”, in **“Examples”**:
  - **Primeros pasos** (first steps): the guide's practice show, with 4 PARs, 2 moving heads and the empty group “Mis escenas”.
  - **Bolo pequeño** (a small gig): PARs at the front and at the back, strobes and a smoke machine. Colours, effects and general looks in three groups, and a SHOW page made with them.
  - **Concierto con cabezas móviles** (a concert with moving heads): colour and position palettes, a mirrored circle and a song as a cuelist on the playbacks.
  - **Sala con playbacks** (a venue with playbacks): one cuelist per song on the first playback page and a SHOW with those playbacks.

The examples open like any show: if you save changes, use “Save as…” to keep the example as it was.

## The program window

The window has three parts: the top bar, the sidebar with the pages, and the page you are using.

### The pages

| Page | Key | What it is for |
|---|---|---|
| Live | F1 | Play the scenes by groups and use the mixer |
| Show | F2 | Your show screen: buttons, faders, dials, groups, clock… |
| Playbacks | F10 | Faders with GO, pause and flash for cuelists and scenes |
| Program | F3 | Select fixtures, give them values, FX and palettes, and record |
| Scenes | F4 | Create and edit scenes, effects, super scenes and cuelists |
| Patch | F5 | The fixtures of the rig, their addresses, groups and 2D plan |
| Fixtures | F6 | Create your own fixture definitions |
| Outputs | F7 | Universes and DMX interfaces |
| Mappings | F8 | MIDI controllers, OSC, DMX input and keyboard |
| Options | F9 | Program settings, fixture library and updates |

**F11** puts the window in full screen or returns it to its size. If a page is locked, it shows a padlock in the sidebar and asks for the password when you open it, if there is one (see [Options](opciones.md#show-and-lock)).

At the bottom of the sidebar are the program version and the **CPU** it uses (“CPU 12 %”, all the computer's cores together). Above 70 % it turns orange, and above 90 %, red: the output can lose its timing. Close other programs or stop some effects.

### The top bar

From left to right:

- **The show**: its name. A dot **●** in front warns you that there are unsaved changes; if autosave is on, you also see the time of the last recovery copy (“recovery copy …”).
- **File**: “New show (Ctrl+N)”, “Open (Ctrl+O)”, the **▾** arrow with the recent shows and the “Examples”, “Save (Ctrl+S)” and “Save as…” (Ctrl+Shift+S).
- **Rhythm**: **“GO”** and **“Stop”** change what the scene buttons do (in Live, the SHOW, the mappings and the phone). With **GO** on, pressing a scene plays it and the next presses mark its rhythm, without ever stopping it; with **Stop** on, pressing a scene stops it; with both off, as usual ([Live](directo.md#rhythm-of-a-scene-with-steps)). Each button of a scene with steps or of a playing cuelist (in Live, the SHOW, the playbacks and the phone) holds a **bar split in two** that lights one side and then the other on each step: this way you see the rhythm of each one. Next come **“BeatGO”** (next division of the scenes driven by BeatGO, also with the G key) and **“Pulse”**, which listens to the audio input of the computer (chosen in [Options](opciones.md#audio)): each peak that crosses the line of the meter moves the scenes driven by Pulse. Drag the line of the meter to change the threshold. The show tempo, used by the FX and by the scenes in BPM mode, is changed with a BPM fader or dial in the [SHOW](show.md) or with the [mappings](mapeos.md).
- **The padlock**: locks screens so that nothing changes by mistake (see [Options](opciones.md#show-and-lock)).
- **The target**: Learn mode, to assign MIDI or OSC controls or keys (see [Mappings](mapeos.md#learn-mode)). Its green light blinks with each message that comes in.
- **“N running”**: how many scenes and effects are playing.
- **GM**: the grand master. Below 100 % it turns orange.
- **“Blackout”** (Ctrl+B): sends everything to black while it is on.
- **“Stop all”** (Esc): stops everything that is playing. The programmer is not cleared.
- **“?”**: opens this manual (F12).

In a narrow window or on a small screen, the bar becomes compact: the show name is cut short, “Save as…” becomes an icon, “N running” keeps only the number and, in the narrowest ones, “Blackout” and “Stop all” keep only their icon. Hover over an icon to see what it is. If everything looks too small or too big, change the **Size of the interface** in [Options](opciones.md#general).

When you close the program, or open or create another show, if there are unsaved changes it asks “Save changes?” with “Cancel”, “Don't save” and “Save”. When it closes, the program first stops every scene, the playbacks and the programmer, and lets the outputs send the stage at rest: many fixtures keep the last thing they received.

If the show you open uses files that are missing (its sounds, videos or images; for example, a show brought from another computer), you are warned with the list. The show plays without them. With **“Look in a folder…”** you choose the folder where they are: they are looked for by name, also in its subfolders, and the show starts using them (save it to keep it that way).

> **Tip:** if the program closed unexpectedly with unsaved changes, when you open it again it offers “Recover the unsaved show?”, with the name of the show and the time of the copy. Press “Recover” and save it (an untitled show, with a name).

## How this manual is organised

Start with **First steps** if this is your first time. The other pages explain each page of the program and can be read in any order.

1. [First steps](primeros-pasos.md): a complete first session, from the patch to saving the show.
2. [Live](directo.md): groups, exclusive or mix, crossfade, masters, releases, flash and mixer.
3. [Show](show.md): the touch screen, its sections, buttons, faders, dials, colour wheel, XY grid, editing and locking.
4. [Playbacks](playbacks.md): playback pages, GO, Pause/Back, Flash, levels, recording on a playback and APC controllers.
5. [Program](programar.md): selection, groups, 2D plan, dimmer, colour, position, features, faders, palettes, FX, fan, blind, highlight and locate, command line, recording and updating.
6. [Scenes](escenas.md): all the scene types, super scenes, cuelists with tracking, folders, import and export.
7. [Patch](patch.md): adding fixtures, addresses, universes, numbers, fixture groups, 2D plan, limits, LED matrices and importing from another show.
8. [Fixtures](fixtures.md): the library, the fixture creator, the Open Fixture Library, GDTF and your fixtures.
9. [Outputs](salidas.md): universes, DMX USB interfaces, Art-Net and sACN, and the DMX levels monitor.
10. [Mappings](mapeos.md): MIDI, OSC, DMX input and keyboard, Learn mode and the templates for Akai APC.
11. [Options](opciones.md): language, theme, size, startup, lock, autosave, priorities, remote control, library, updates.
12. [Remote control](remoto.md): the SHOW on your phone or tablet.
13. [Keyboard shortcuts](atajos.md): all the keys and combinations.

## Words you will see often

- **Fixture**: each patched light (a PAR, a moving head, an LED strip…).
- **Universe**: 512 DMX channels. Each universe goes out through an interface.
- **Programmer**: where you give values to the selected fixtures. What is in the programmer overrides everything until you clear it.
- **Scene**: what you record to play it back later. There are several types: static, steps, FX, super scene, cuelist… (see [Scenes](escenas.md)).
- **Group**: a column of scenes in Live. In an exclusive group only one scene plays at a time.
- **Fixture group**: a set of fixtures to select them in one go (they are created in [Patch](patch.md#fixture-groups)).
- **Cuelist**: a list of cues that advance with GO. Each cue stores only what changes.
- **Playback**: a fader with its buttons (GO, Pause/Back, Flash) that plays a cuelist or a scene.
- **Palette**: a stored value (a colour, a position…) that scenes follow by reference: if you change the palette, the scenes that use it change too.
- **Priority**: from 0 to 10. If two scenes control the same channel, the one with the higher priority wins.
- **SHOW**: the operating screen you build yourself.
