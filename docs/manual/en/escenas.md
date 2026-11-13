# Scenes

![The Scenes page](../img/escenas.png)

**Scenes** (F4) is the library of everything you have recorded: static scenes, steps, FX, colour effects, racks, super scenes and cuelists. The list is on the left; on the right, the editor of the chosen scene, which depends on its type.

Almost everything is created from the programmer: new effects take the fixtures selected in [Program](programar.md), and steps and cues take its values.

## The scene list

- **“Search…”**: filters by name (or by the name of the folder).
- **“Folders”** / **“Groups”**: shows the scenes by [folders](escenas.md#folders) or by the Live groups they are in (“Not in a group” for the rest).
- **Fold all** / **Unfold all**: the button on the right.
- **Type filter**: “All”, “Static”, “Steps”, “Cuelists”, “FX”, “Colour FX”, “Racks” and “Super scenes”.
- **Ctrl+click** and **Shift+click** select several scenes to move them to a folder (“To folder…”).

Each scene has a stripe with the colour of its type. Click one to open it in the editor.

## Creating a scene

Press **“New”**:

| Option | What it creates |
|---|---|
| **Static (from the programmer)** | A static scene with the values of the programmer. |
| **Steps (programmer = first step)** | A steps scene. |
| **Cuelist (programmer = cue 1)** | A cuelist. |
| **FX (in the programmer)** | Takes you to Program › FX: FX are made there and recorded with “Record…”. |
| **Colour FX (selected fixtures)** | A colour effect on the selected fixtures, in their order. |
| **Mapping on the 2D plan (selected fixtures)** | A colour effect laid out where the fixtures are on the 2D plan. |
| **Effect rack** | An empty rack. |
| **Super scene (timeline with audio)** | An empty super scene. |
| **Import from a file…** | Scenes and effects exported from another show (see [Import and export](escenas.md#import-and-export)). |
| **QLC+ cue lists…** | Cuelists from the chasers, sequences and cue lists of a QLC+ show (see [QLC+ cue lists](escenas.md#qlc-cue-lists)). |

## The editor header

- **The name**: click it to change it.
- **The folder** (folder icon): moves the scene to a folder.
- **“Try”** / **“Stop”**: plays it now, outside the groups. Cuelists have their own GO.
- **“Add to group…”**: adds it to a Live group.
- **Convert to steps** (racks, FX, Colour FX, Move FX, Curve FX and chasers): records one cycle into a new steps scene, with only the channels the scene gives a value to: the others, even if they have a default value (the pan or the zoom in the centre on many moving heads), do not go into the steps. If an FX has delays, it records the cycle once all its fixtures have started.
- **Convert to a cuelist** (steps and chasers): see [Cuelists](escenas.md#cuelists).
- **“QLC+”** (cuelists): saves a copy of the show for QLC+; see [Cuelists and QLC+](escenas.md#cuelists-and-qlc).
- **Export**, **“Duplicate”** and **“Delete”** (it is also removed from its groups and racks).

## How a scene plays

Under the header is the playback panel. It shows what makes sense for each type:

- **“Fade in”** and **“Fade out”**.
- **“Cycle time”** (or **“Step time”** on a Colour FX) and **“Speed”** (from one cycle a minute to ten a second; double click: one cycle a second), with **×½** and **×2**.
- **“Staggered fade”** (static and steps): each fixture starts its fade this long after the previous one, in selection order.
- **“Driven by”**: **“Duration”** (the normal times), **“BPM”** (the times become beats and follow the show tempo, the fades of each step too; the crossfade time of a Live group and the fades of the blocks of a super scene stay in seconds), **“BeatGO”** or **“Pulse”** (it advances with the BeatGO button or with the peaks of the audio input). With BeatGO or Pulse, **“Divisions”** (/2, ×2).
- **“Tapped rhythm”** (sequences, chasers and FX scenes with FX at “BPM of the show”): the rhythm tapped by pressing its button several times in a row ([Live](directo.md#rhythm-of-a-scene-with-steps)), in BPM. While it has one, its steps go at that rhythm and not by their times; in an FX scene, its FX at “BPM of the show” do one cycle on each beat instead of following the BPM of the show. **“Clear”** goes back to its own times (or to the BPM of the show). With “BPM”, a beat is a step of that rhythm: a 0.5 b fade lasts half a step.
- **“Start”**: **“Playing”** or **“Paused, press again to play”**.
- **“Plays”**: **“Always”** or **“N times”**, **“then”** **“Release it”** or **“Hold the last frame”**, **“and play”** “Nothing”, “Next scene of the group”, “Previous scene of the group” or any other scene.

## Scene types

### Static

It stores fixed values. It shows how many channels and fixtures it has and the palettes it uses (“Uses these palettes: when they change, the scene changes too”).

To change it:

1. Press **“Edit in the programmer”**: it is loaded into Program.
2. Change what you want.
3. Go back to Scenes and press **“Save the programmer in this scene”**.

If you save FX in a static scene, it becomes an FX scene.

### Steps

A sequence of looks, each one with its **fade in**, its **hold** and its **fade out**.

- **“Add step”**: the programmer becomes a new step after the selected one.
- **“Save in step”**: saves the programmer in the selected step.
- **“Load step”** (or double click a step): takes it to the programmer to see and change it.
- **Duplicate**, **▲ ▼** to reorder and **delete** the step.
- The **∞** button of the hold makes the step wait for a GO (“Until GO”).

**Ctrl+click** and **Shift+click** select several steps: when you change a time, it changes on all of them. Steps use palettes **by reference**, like scenes and cues: when you update a palette, the steps recorded with it change (the one playing, at once). The palette icon of a step tells which ones it uses.

### FX

An FX scene has **base values** and one or more **FX layers**. It is created by recording from the programmer with FX (see [FX](programar.md#fx)).

- On the left, the list of FX: parameter, fixtures and speed (at “BPM of the show”, the tapped rhythm of the scene if it has one). The bin removes a layer.
- On the right, the controls of the chosen layer (the same as in Program) and its animated preview. You see the changes live while the scene plays.
- **“Link”**: linked layers share speed and timing.
- If some fixtures do the FX their own way, you are warned: if you change the shape, the size, the direction or the phase here, all of them will do the same.
- **“Per fixture”**: the shift and the delay of each fixture, live too. Here “Fan…” and “No shift or delay” go over all of them (see [Per-fixture shift and delay](programar.md#per-fixture-shift-and-delay)).
- To add layers or change the base values: **“Edit in the programmer”** and then **“Save the programmer in this scene”**.

### Colour FX and mappings

A **Colour FX** is a pixel effect (more than 60: gradients, fire, meteors, strobes, waves…) that runs through the fixtures in selection order.

- **“Effect”**: which one.
- **“Works on”**: “Colour (RGB)”, “White”, “Amber”, “UV”, “Dimmer (value)” or “Shutter”.
- **“Colours”**: up to 5, depending on the effect. Click a colour to change it (swatches or hue), or “No colour”.
- **“Text”**: in text effects, the text that runs through the fixtures.
- **“Image”** and **“Animation”**: in image effects, the file and “Still”, “Scroll sideways”, “Scroll up and down” or “Animated (GIF)”.
- **“Layout”**: **“In a row”** (in selection order) or **“On the 2D plan”** (where the fixtures are).
- **“Parameters”**: the ones of each effect.
- **“Use the programmer selection”**: swaps its fixtures for the selected ones.

At the top you see the real colour of each head while it plays (press “Try” to see it).

A **mapping on the 2D plan** is a Colour FX that already comes with the plan layout: the effect, the text or the image are laid over the stage just as the fixtures are placed.

### Move FX and Curve FX

They are effects from the previous module: **Move FX** move pan/tilt with a shape and **Curve FX** give a wave (sine, triangle, ramps, square, random, strobe…) to channels. You will find them in old shows or in QLC+ shows; they are no longer created from “New”.

They still play the same and have their own editor (shape, width, height, rotation, position, phase, spread over the fixtures, symmetric, relative…). A notice offers **“Convert to FX”**: they move to the FX editor, also in the programmer, and the groups, SHOW buttons and mappings keep them. The phase of a Move FX becomes the “Phase” of the FX, and the start offset of each fixture becomes its “Shift” ([Per-fixture shift and delay](programar.md#per-fixture-shift-and-delay)).

### Effect rack

Several scenes and effects that play together as one, each one on its own fixtures.

- In **“Add”** (on the right), click a scene to put it in the rack.
- In **“In the rack (N)”**, the **×** removes it.

A rack cannot contain a scene that plays the rack itself.

### Chasers

Chasers from QLC+ shows play normally, but they have no editor: convert them to steps (it records one cycle) or to a cuelist with the buttons in the header.

## Super scenes

A **super scene** is a timeline with scene tracks and audio tracks. You play it like any scene (in a group, in the SHOW, with a mapping). Its editor takes up the whole page; the list button, at the top left, goes back to the scene list.

### The toolbar

- **“Play”** / **“Stop”** (**Space**): from the cursor. **⏮** goes back to the start. Next to it you see the time (and the bar).
- The **Undo** and **Redo** arrows (**Ctrl+Z** / **Ctrl+Y**). See [Undo](escenas.md#undo).
- **“Paste”** (**Ctrl+V**): the blocks copied, at the cursor and from the selected track. See [Copy and paste](escenas.md#copy-and-paste).
- **“Time”** / **“Bars”**: the grid. In bars, the **BPM**, the time signature (**4/4**, **3/4**, **2/4**) and **“Bar 1 here”** so that the first bar starts at the cursor and lines up with the music.
- **“Snap”**: blocks and points snap to the grid.
- **“End here”**: the super scene ends at the cursor (the **×** removes the marker: it ends with its last block). **“Loop”**: it starts again.
- **“+ Track”** and **“+ Audio”** (an audio file on the selected track, at the cursor).
- **“Static values”**: **“Take the values of the programmer”** (they play while the super scene is running) or **“Clear the static values”**.
- **Zoom** − and +.

### Putting scenes on the timeline

1. Press **“+ Track”** if there is none.
2. Drag a scene from the list on the left onto a track. Or select a track, place the cursor and **double-click** the scene: it is placed at the cursor.

Blocks can overlap.

A **“Timecode”** cuelist with its own clock in a block follows the timeline: the 00:00 of its clock is the start of the block. Put it on another track, next to the audio block of the song and from the same point: each cue plays at its moment of the song. If you play from another point, the cuelist goes straight to the cue of that time. Stretched, its clock goes faster or slower so that the cuelist lasts the block.

### Working with blocks

- **Drag** a block to move it, also to another track. Drag its **edges** to change its length.
- With a single block selected, the **handles** on the top corners are its fade in and its fade out.
- When you lengthen a block, the scene **loops** (you see marks on each lap) or is **stretched** so that one cycle lasts the whole block. You change this in the block menu or in the bar at the bottom (“Loops” / “Stretched”).
- **Right click**: “Loop the scene in the block” / “Stretch the scene to fit the block”, “Copy”, “Cut”, “Duplicate”, “Lock” / “Unlock”, “Open the scene” and “Delete”. On a block of the selection, the menu acts on the whole selection; “Open the scene” opens the scene of the block clicked.
- A **double click** on a block opens its scene. **Del** (or **Backspace**) deletes the selected blocks; those on locked tracks stay. **Ctrl+D** duplicates them right after themselves: each copy on its own track.
- The bar at the bottom shows the selected block: **Start**, **Length**, **Fade in**, **Fade out**. With several, it says how many there are. With one or several, it has **Copy**, **Cut**, **Duplicate** and **Delete**.

### Selecting several blocks

- **Click** a block: only that one. Its track becomes the selected track.
- **Ctrl+click** adds a block to the selection or takes it out.
- **Shift+click** selects from the last block chosen to this one: the blocks on the tracks between the two, from the earlier start to the later end.
- **Dragging on an empty part** of the tracks draws a rectangle that selects the blocks it touches. With **Ctrl** or **Shift**, they are added to the selection.
- **Ctrl+A** selects every block on the tracks in view. The blocks of a track you hide leave the selection.
- A click on an empty part clears the selection (and moves the cursor). With Ctrl or Shift, the selection stays.

When you **drag** a selected block, the whole selection moves:

- They all move by the same amount in time: the block dragged snaps to the grid and the others keep their distances. The first one stops at 0.
- They go up or down by the same number of tracks. The tracks they go to light up.
- Locked blocks and blocks on locked tracks stay where they are.
- If any of them would land outside the tracks or on a locked track, they move only in time.
- Edges and fades change only the block under the mouse.

Pressing a block of the selection and releasing it without moving leaves only that block selected. Each action on the selection is a single [Undo](escenas.md#undo) step (“Move 3 blocks”).

### Copy and paste

- **Ctrl+C** copies the selected blocks. **Ctrl+X** cuts them: it copies them and takes them away. Those on locked tracks are copied, but they stay.
- **Ctrl+V** or **“Paste”** pastes them at the cursor, from the selected track (with no track selected, from the first one).
- **Right click** on an empty part of a track: **“Paste here”** pastes at that point and on that track. **“Select all”** does the same as **Ctrl+A**.
- The first block lands at the cursor and the others keep their distances in time. The one on the topmost track goes to the selected track and the others keep their distances in tracks.
- Missing tracks are added at the end. A hidden track that gets blocks is shown.
- Pasted blocks keep their length, their fades, loop or stretch and their colour. They come unlocked, as with Duplicate.
- What you paste stays selected and the cursor moves to its end: press **Ctrl+V** again and the next copy goes right after it.
- You can paste into another super scene of the same show: open it and paste. Scenes are not copied: the pasted block plays the same scene.
- An audio block pasted in the same super scene plays the same audio. In another super scene it gets its own audio, read from the same file: deleting it there never takes the original. If you cut the block, its audio is read again from the file.
- Nothing is pasted on a locked track, nor a scene that plays this super scene (for example, a rack that contains it). A message says so.
- What you copy is kept while the show is open. Opening another show or starting a new one empties it. A scene you delete from the list leaves it.
- Pasting and cutting are a single [Undo](escenas.md#undo) step. Blocks pasted while the super scene plays are heard from the next “Play”.

### Audio

Audio blocks (wav, flac, ogg, aiff, and mp3 if the system supports it) show their **waveform**, which is worked out in the background. You need an unlocked track to add one.

### Tracks and automation

In the header of each track:

- **M**: mute (the track does not play).
- **Padlock**: lock (its blocks and points do not move).
- **D**: the **dimmer** automation lane (on an audio track, the volume).
- **φ**: the automation lane for the **phase** of its effects.
- **⋯**: “Rename” (also with a double click on the name), “Move up”, “Move down”, “Clear the automation of this lane” and “Delete the track”.
- The arrow on the right hides or shows the track.

On an automation lane: a **click** adds a point, **dragging** moves it, a **double click** deletes it and a **right click** switches between a straight line and a smooth curve. With no points, the dimmer is at full and the phase at 0.

### Moving around the timeline

- Clicking or dragging on the **ruler** moves the cursor; a **double click** on the ruler plays from there. A click on an empty track also moves the cursor.
- **Ctrl+wheel**: zoom. **Shift+wheel**: move through time. The wheel on its own scrolls up and down through the tracks.

### Undo

- **Ctrl+Z** undoes the last change. **Ctrl+Y** (or **Ctrl+Shift+Z**) redoes it. The arrows on the toolbar do the same; hover over them to see which change they undo or redo.
- Each super scene has its own history, of up to 100 steps. It lasts while the show is open: saving doesn't clear it, opening another show or starting a new one does.
- Changes to tracks, blocks, automation, grid, BPM, end, loop and static values count. Several changes of the same field of the bottom bar (or of the BPM) within one second are a single step.
- Hiding or showing tracks, zoom, the cursor, “Snap” and the selection don't count.
- While you type in a field, Ctrl+Z undoes what you typed in the field.
- It works while the super scene plays. A block taken away stops; a block that comes back plays from the next “Play”.
- An audio block comes back with its audio, read again from its file.
- The blocks of a scene deleted from the list don't come back.

## Cuelists

A **cuelist** is a list of cues that advance with GO. It works with **tracking**: each cue stores **only what changes** and the rest carries on from the previous cues. A GO only changes the channels that change: no dips, and nothing it does not touch is retaken.

With other cuelists and scenes of the same priority, each channel that is not an intensity (position, gobo, colour wheel…) comes from the last one that changed it: the GO that changes it takes it, and what the cuelist only keeps from earlier cues is not taken back. When it is released, it fades to the one that had it before. See [Priorities](directo.md#priorities).

Cuelists are recorded from the programmer, run in [Playbacks](playbacks.md) and edited here.

### Cuelist options

At the top of the editor:

- **“Type”**: how the cuelist plays. A line next to the buttons says what the chosen type does. In Playbacks and in this editor, a cuelist of another type shows its tag: **SUB**, **CHASE**, **TC** or **OVR**.
  - **“Normal”** (the default): each GO plays the next cue.
  - **“Chase”**: the cues follow each other by themselves, in a loop, at a speed (see below).
  - **“Timecode”**: each cue plays at its time on a clock (see below).
  - **“Submaster”**: the fader of its playback rules it (see below).
  - **“Override”**: its fader crossfades everything it holds between what is below and its cue (see below).
- **“Tracking”**: on, each cue stores only what changes. Off, each cue stands on its own.
- **“After the last cue”**: **“Loop”** (back to the first one), **“Release”** or **“Stay”**.
- **“Release time”**: the fade when the cuelist is released.
- **“Reset on release”**: after it is released, GO starts again from the first cue.
- **“Released by another GO”**: a song that ends when the next one starts.
- **“New cue fade”**: the fade of the first recorded cue; the next ones take the times of the previous one.
- **“Back”**: the BACK times: **“Cue times”** or **“Fixed time”**.
- **“Priority”**.
- **“GO on the beat”**: “Off”, “BeatGO” or “Pulse” give a GO on each division.
- **“Tapped rhythm”**: the rhythm tapped by pressing its button several times in a row, in BPM: a GO on each beat. While it has one, its Follow and Wait times do not count. **“Clear”** goes back to its own times.
- **“Move in black”**: when a cue has finished, the dark fixtures that the next cue lights up already take its position, colour and beam. At GO they only fade up. **“After”** is the wait after the end of the cue (0 s by default). **“Time”** is how long those moves take (2 s by default).
  - A fixture that another playback or the cue itself lights up is not moved.
  - On a fixture without a dimmer, its RGB is its light: only its other attributes are prepared.
  - The channels of an FX are not prepared: the FX starts at its GO.
  - If you choose another cue for the next GO (“Next” in [Playbacks](playbacks.md)), that one is prepared.
  - It doesn’t apply in a “Chase” or “Override” cuelist.

In a narrow window, some of the options are folded away: **“More options”** shows them.

A **“Submaster”** cuelist is ruled by the fader of its [playback](playbacks.md#the-fader):

- When you raise the fader from zero, it starts on its next cue, at the level of the fader and never at full.
- When you lower it to zero, it is released with its release time. You can change this in the [properties](playbacks.md#playback-properties) of the playback.
- While it plays, GO and Back change the cue at the level of the fader.
- While it is released, GO and Back only choose the cue it will start on (Pause/Back and “Go” too). The playback shows it as the next one (“▸ 3”). After the last cue, GO goes back to the first one; with “Stay”, it stays on the last one.
- If you release it with the fader up, lower the fader to zero and raise it again to start it.
- The flash takes it to full while you press it.
- In a group of Live and on a SHOW button it plays as a normal cuelist. It does too when it is on no playback, or when its fader doesn't start it (“The fader starts it” off).

A **“Chase”** cuelist plays its cues by itself, in a loop:

- **“Speed”**: a step on each beat, in BPM (from 10 to 600; 120 by default). With **“BPM of the show”**, it follows the tempo of the show. **×¼**, **×½**, **×1**, **×2** and **×4** multiply it.
- **“Crossfade”**: the part of the step the change takes, in %. With 0 (the default), it is a cut. With **“Cue times”**, each cue uses its own delay and fade.
- **“Direction”**: **“Forward”**, **“Backward”**, **“Bounce”** (it turns at the first and the last cue) or **“Random”** (it never plays a step twice in a row).
- GO goes to the next step at once, and the next steps count from there. Back goes one step the other way. Pause holds it where it is.
- A **“Tapped rhythm”** wins over its speed; **“Clear”** goes back to it. With BeatGO or Pulse, it only steps on each division.
- The [cue speed](playbacks.md#cue-speed) and the live speed also make it faster or slower.
- Tracking still applies: a step can change only the colour and keep the rest.
- It always goes round: it has no **TRIGGER** column and no **“After the last cue”**, and the links of the cues do not count. With a crossfade in %, the times of the cues are dimmed: they are not used.
- In a super scene, a block lasts its steps times the length of a step. With “Bounce”, there and back.

In an **“Override”** cuelist, the fader of its [playback](playbacks.md#the-fader) mixes it with what is below:

- The fader crossfades everything it holds: intensities, colours, positions and gobos. At half way, the moving heads are half way. At zero, everything is as if it weren't there.
- At full, its cue replaces what is below, on the intensities too: a blue over a red gives blue, not magenta.
- Cues, GOs and fades work as in a normal cuelist. The fade of a cue and the fader multiply.
- When you release it, everything fades back to what is below in its release time. With the fader at zero it stays loaded, like any cuelist.
- When you choose “Override”, a cuelist at the default priority (5) goes to 10. The programmer stays above it.
- A cuelist or a scene of the same priority started after it mixes on top as usual: on the intensities the highest value wins.
- A channel with **Fades** off in the [fixture limits](patch.md#limits) jumps to its value as soon as the fader leaves zero.
- Its FX play as in a normal cuelist: the fader does not crossfade them.
- If the cuelist is already playing, a change to or from “Override” counts from its next start.
- Typical uses: a blinder or the house lights, a fixed position for a solo, an emergency light on a fader.

A **“Timecode”** cuelist plays each cue at its time on a clock:

- Each cue has its **TIME** (hours:minutes:seconds.hundredths). The **TIME** column replaces **TRIGGER**. Type the time (“00:01:23.40”, “1:23.4” or “83.4”) or use the wheel. If you clear it, the cue has no time and the clock does not play it.
- **“Clock”**: **“Own clock”** or **“MIDI timecode (MTC)”** (see below). With its own clock, GO starts the cuelist and its clock at 00:00, or at the time of the cue it is on (the one chosen with “Go to cue” or, with “Reset on release” off, the next one after a release).
- The last cue of the list whose time has come plays. Before the time of the first cue, the cuelist shows nothing.
- When it moves on to the next cue, the cue makes its fade. A jump (back, or over several cues) goes straight to the look of that time, with no fade.
- GO goes to the next cue with its fade, and the clock jumps to its time. Back goes to the previous cue and its time, with no fade (with a fixed time in **“Back”**, in that time). “Go to cue” sets the clock to its time. With a cue that has no time, the clock goes on as it was.
- Pause stops the clock. The [cue speed](playbacks.md#cue-speed) and the live speed make it faster or slower.
- Its Follow, Wait and links do not count. “After the last cue” applies to GO on the last cue.
- The header and its playback show the clock: **“TC 00:01:23.4”**.
- **“Set to the clock”** (the clock in the toolbar) puts the selected cue at the time of the clock while the cuelist plays.
- A cue recorded while the clock runs takes its time. With the cuelist released or paused, it takes the time of the previous cue plus 5 s.
- A triangle warns of a cue whose time is before the time of the cue before it. Of the two, only the last one plays.
- When you choose “Timecode”, the cues without a time get one: 00:00 for the first one, and 5 s more than the previous one for the others.
- In a [super scene](escenas.md#super-scenes), the clock is the timeline.

With **“MIDI timecode (MTC)”**, the cuelist follows the timecode that a DAW, a video player or a timecode generator sends to a MIDI input:

- Turn on the input of the MIDI device in [Mappings › Devices](mapeos.md#devices). No mapping is needed.
- **“Offset”**: the timecode where the 00:00 of the cuelist is, in hours:minutes:seconds:frames (for example “01:00:00:00”). Before that time, the cuelist shows nothing.
- Next to it you see the timecode that comes in, **“MTC 01:00:12:05 · 25 fps”**, or **“No MIDI timecode”**.
- Frame rates: 24, 25, 29.97 drop-frame and 30.
- When you start it, the cuelist waits for the timecode and goes straight to the cue of that time. If it is the first cue, the cue makes its fade.
- If the timecode stops or drops out, the cuelist stays on its cue. If it comes back from another time (a rewind or a jump), the cuelist goes straight to the cue of that time, with no fade.
- GO, Back and “Go to cue” do nothing: the timecode is in charge. GO on the released cuelist starts it.
- The cue speed doesn't move the clock; it only changes the fades.
- In a super scene too, the clock is the timecode.

### The cue table

| Column | What it is |
|---|---|
| **CUE** | The number. It accepts decimals: 2.5 goes between 2 and 3. Double-click it to change it. A pencil shows that the cue is loaded in the programmer. |
| **NAME** | Its name. |
| **TRIGGER** | **GO** (waits for a GO), **Follow** (when the previous cue has finished, plus a time) or **Wait** (a time after the GO of the previous cue). |
| **TIME** | Only in a “Timecode” cuelist, instead of TRIGGER: the time of the cue on the clock. A triangle warns when it is before the time of the cue before it. |
| **DELAY** | How long the cue waits before starting its fade. |
| **FADE** | The fade time. |
| **DOWN** | The fade of the dimmers that go down. “=” is the same as the fade. |
| **CHANGES** | How many channels it changes, and the **FX** and **[R]** (released values) marks, the palette icon if it uses palettes, a clock if it has its own times or a curve, a chain link if it has a link and a bolt if it has actions (hover over them to see them). |
| **S** | **Skip** (cue only): what it changes does not go on to the next cues. |
| **B** | **Block**: Record and Update also store the values that are equal to the ones it tracks. |
| **MIB** | Only with “Move in black”: the cue is prepared while its fixtures are dark. Unticked, its moves are seen at its GO. |
| **NOTE** | A note for you. |

To change a time, click it and type the seconds, or use the mouse **wheel** over the clicked time: ±0.1 s (with Shift, 1 s). With **Ctrl+click** or **Shift+click** you select several cues and change their times, their trigger or their marks at once.

The playing cue has a green bar with the progress of the fade; the next one, a green border. At the top you see the state (“Q 3 · Verse ▸ 4”, with “↺ 1/3” when the next cue is a jump of a link, or “Released · GO plays 1”) and the **Back**, **GO** and **release** buttons, the same ones as on its playback.

While a cue is fading, the header also shows **“Live time”**: the time of that cue, which you can change while it runs (by typing another one, with **“×½”**, **“×2”** or **“Now”**), and the button that stores it in the cue. It works as in [Playbacks](playbacks.md#live-time).

### Recording and editing cues

- **“Record cue N”**: the programmer becomes a new cue after the selected one, with only what changes compared with what it tracks.
- **“Update ▾”**: merges the programmer into the selected cue: **“Update cue N (the next cues follow)”** or **“Only this cue (the next cues keep their look)”**.
- **“Load ▾”** (or double click a cue): takes it to the programmer: **“What the stage shows on cue N”** or **“Only what cue N changes”**. Then update it from the “Editing cue…” bar in Program.
- **“Take out ▾”**: with the channels of the programmer, **“Remove the programmer's channels from cue N (they track again)”** or **“[Release] the programmer's channels in cue N”** (from that cue on, they go back to what is underneath: other scenes or their rest value).
- **Copy** to the end: **“Copy what cue N changes”** or **“Copy the whole look of cue N (block cue)”**.
- **“Number…”**: another number for the selected cue. Moving a cue changes what the cues after it track.
- **Delete**: with **“Keep the look of the next cues”** ticked, what the cue changes goes on to the next one and the cues after it look the same.

In a narrow list (or in a language with long words), “Update”, “Load” and “Take out” keep only their icon and their **▾** arrow; hover over them to see their name.

The right click menu on a cue has **“Go to cue N”**, the two ways to load, **“Update with the programmer”**, **“Times…”**, **“Link and actions…”**, **“Number…”** and **“Delete…”**.

Cues can use **palettes by reference** and carry **FX layers**, which track from cue to cue.

### The tracking sheet

**“Cue details”** has three tabs: **“Tracking”**, **“Times”** and **“Link and actions”**. If the window is wide, the panel shows by itself next to the cues.

**“Tracking”** shows what the stage has on the selected cue, fixture by fixture (“Cue N on stage”).

- The coloured tags tell you what each value is: **“Changes”** (this cue changes it), **“Tracked”** (it comes from a previous cue, which is shown: “Q2”), **“Block”** and **“Released”** (struck through, with [R]).
- At the top you see the palettes the cue uses.
- Click channels to pick them and press **“Release”** (puts them in [RELEASE] from this cue on) or **“Take out”** (they track again from the previous cues).

### Times of a cue

Inside a cue, each attribute and each fixture can have its own delay and its own fade. Select a cue (or several) and open **“Cue details”** › **“Times”**, or right click › **“Times…”**. With several cues selected, the changes apply to all of them.

- **“Times per attribute”**: **Intensity**, **Colour**, **Position** and **Beam**, each with DELAY and FADE. Empty or “=” is the time of the cue. The strobe goes with Intensity; gobo, prism, zoom, focus and iris go with Beam.
- **“Times per fixture”**: **“+ Selected fixtures”** adds the fixtures selected in Program, with the times of the cue. Then change their times. **×** removes one and **“Clear”** removes all of them.
- The time of a fixture wins over the time of its attribute, and the time of an attribute wins over the time of the cue.
- Dimmers going down take the fade of their fixture, if it has one. If not, the DOWN time of the cue; with “=”, the Intensity fade.
- A cue lasts as long as its longest time: Follow, the green bar and the length in a super scene count it. The time of an attribute that the cue doesn't change counts too.
- One time for everything, as before: the fade in of a group or of a super scene block on the first cue, and a fixed **“Back”** time. With **“Cue times”**, BACK uses all the times of the cue it goes back to.
- FX layers fade in with the fade of the cue.
- A new cue takes the attribute times of the cue before it, but not its fixture times. Copying a cue copies all its times.

Example: cue 2 changes colour and position with fade 0, and Position has delay 1 s and fade 3 s. On GO, the colour changes at once; a second later, the moving heads move for 3 s.

**Curves.** **“Curve of the fades”** sets how the fades of the cue move:

- **Linear** (default): the same speed all the way.
- **Smooth**: slow at the start and at the end.
- **Slow start**: it barely moves at first and speeds up at the end.
- **Fast start**: it moves almost at once, then settles.

The curve applies to everything the cue fades: dimmers going up and down, colours, moves and the channels it releases. Each attribute also has its **CURVE**; “=” is the curve of the cue. The release time of the cuelist, a fixed **“Back”** time, the fade in of a group and FX layers are always linear. The clock in the CHANGES column also shows when the curve of the cue is not linear.

**Time fan.** In **“Times per fixture”**, **“Fan”** spreads a delay or a fade over several fixtures:

- Choose **Delay** or **Fade**, and the times **from** and **to**.
- Choose the shape: **One after the other**, **From the middle** or **From the ends**.
- Choose the fixtures: **“Selected in Program”**, in the order you selected them, or **“Of this cue, by number”**.
- **“Apply”** gives each fixture its time in the selected cues. Its other time does not change.

With 5 fixtures and a delay from 0 to 2 s, **One after the other** gives 0, 0.5, 1, 1.5 and 2 s. **From the middle** gives 2, 1, 0, 1 and 2 s. **From the ends** gives 0, 1, 2, 1 and 0 s. If “from” is greater than “to”, the order is reversed. The fan writes fixed times, not a rule: you can then change each row.

### Links and actions

Select a cue and open **“Cue details”** › **“Link and actions”**, or right click › **“Link and actions…”**. With several cues selected, you edit the last one you picked.

**Link.** **“After this cue”** says which cue plays after it: **“The next cue”** (the usual) or another cue of the list. **“Times”** says how many times the link jumps before the list goes on; 0 is always. A new link starts at 1.

- With cue 5 → cue 2, 3 times, the GOs play 1 2 3 4 5, 2 3 4 5, 2 3 4 5, 2 3 4 5 and 6: cues 2 to 5 play 4 times. Below, a sentence spells it out: “Cues 2 to 5 play 4 times, then cue 6.”
- With 0 (always), the cues repeat until you go to another cue.
- A forward link skips cues; a link to the cue itself repeats it.
- The trigger of the cue it jumps to applies: if cue 2 has Follow, the loop goes on by itself.
- That cue is the next one everywhere: the green border of the table, the header (“▸ 2 ↺ 1/3”: the first jump of three; only “↺” when always), the playbacks, the SHOW, the phone and the LEDs of the controller.
- BACK and “Go to cue” don’t count jumps. Releasing the cuelist, or starting it again, starts the count again.
- Once all the jumps are done, the list goes on past the cue. The next time it gets there, the count starts again.
- On the last cue, the link comes before **“After the last cue”**.
- In a “Chase” or “Timecode” cuelist, links don’t count: the tab says so. The link is kept for when the list is “Normal” again.
- If you delete the cue it jumps to, the link goes and a message says so.

**Actions when it starts.** **“+ Add an action ▾”** adds what the cue does when it starts:

- **“GO of a cuelist”**, **“Go to a cue…”** of another cuelist and **“Release a cuelist”**.
- **“Start a scene”** and **“Stop a scene”**, in its group if it has one (as the SHOW buttons do). They don’t change the selected scene.
- **“Release a group”** (or all of them) and **“Release all playbacks”**.
- **“Set the BPM”**, from 20 to 300.

Each row is changed in place; **×** removes it. The actions run in order when the cue starts with GO, Follow, Wait or “Go to cue”, also the first cue when the cuelist starts. They don’t run with BACK, when an edit applies the cue again, nor when a super scene starts the cuelist halfway.

- The cuelist itself is not in the list: the link does that.
- An action on a deleted scene or cuelist shows “(deleted)” and does nothing.
- If two cuelists start each other, the loop stops: a cuelist started by an action doesn’t start, with its actions, a cuelist earlier in that chain. Also, the same cue doesn’t repeat its actions within 200 ms.
- Copying a cue doesn’t copy its link nor its actions.

### Undo

The **Undo** and **Redo** arrows, after the bin, undo and redo the edits of the cuelist, one by one. So do **Ctrl+Z** and **Ctrl+Y** (or **Ctrl+Shift+Z**). Hover over them to see what they undo: “Undo: Record cue 3”.

- Everything that changes the cues or the options of the cuelist can be undone: recording (also from Program, a playback or the command line), updating, taking out, releasing, copying, deleting, the number, the times, the trigger, the name, the note, the S and B marks, the link, the actions and the options at the top, except “GO on the beat” and “Tapped rhythm”.
- Each cuelist keeps its last 50 steps.
- Several wheel notches (or typed values) on the same time, less than 2 seconds apart, are one step. If you go back to the value you started from, the step disappears.
- After an undo, the first cue that changes is selected. If the cuelist is playing, the stage changes at once.
- In a field with typed text, Ctrl+Z undoes that text first.
- Creating the cuelist and converting steps to a cuelist can't be undone. An undo keeps the current values of the palettes and doesn't bring back the channels of deleted fixtures.
- A priority or a release time changed elsewhere (Live, Playbacks) stays as it is.
- The history isn't saved with the show. New and Open empty it, and so does deleting the cuelist.

### Converting steps or a chaser to a cuelist

In a steps scene, the **“Convert to a cuelist”** button in the header turns each step into a cue with what changes from the previous one; steps with a time are chained with **Wait**. Its groups, SHOW buttons and mappings now play the cuelist. The steps disappear. The cues keep using the palettes of the steps.

In a chaser, each cue holds the scene of its step (Move FX become FX) and releases what the step before had and this one doesn't; steps that are neither scenes nor effects give an empty cue with a note. Its scenes stay.

A ping-pong goes there and back; a random order plays in order. Times in beats become seconds at the BPM of that moment. The steps of a copy for QLC+ come back with the number, the name and the note of each cue.

### Cuelists and QLC+

QLC+ does not read cuelists: if you open this show in QLC+, they are not there. To take them to QLC+, press **“QLC+”** in the header of a cuelist. **“Save a copy for QLC+”** writes another `.qxw` in which each cuelist checked is a QLC+ step list:

- **Sequence** (the default): one item in QLC+.
- **Chaser**: a scene for each cue, in a folder named like the cuelist.

With **“A cue list for each one in the Virtual Console”** checked, the copy opens on the QLC+ Virtual Console, with a “Cuelists” frame and a cue list for each cuelist: “Next” is the GO, “Previous” the BACK and “Stop” releases it. Your show does not change.

The **“With their keys and controller buttons”** box gives each cue list the keys and controller buttons of its cuelist: GO, Back, Pause/Back and Release become “Next”, “Previous”, “Playback” and “Stop”. The mappings of the cuelist count, and those of the playback that holds it on the current page. Each QLC+ button takes one key and one input. OSC controls and those of a device that is not enabled do not go; the window says so.

Each step holds everything the stage shows on that cue, and its note gives the number and the name (“Q2.5 · Verse”). The fade of the cue becomes the fade in of the step, and the down fade its fade out. GO leaves the step waiting for “Next”; Wait and Follow become a time. In a “Chase” cuelist, the steps go on by themselves, in a loop and in its direction: each one lasts a step of its speed, with its fade. In a “Timecode” cuelist, each step lasts until the time of the next cue. The steps keep the ID of the cuelist: the super scenes and racks that play it play the steps.

Under each cuelist, the window tells you what does not go:

- “Chase” with “BPM of the show” or a tapped rhythm: the steps keep a fixed BPM, times its multiplier: the show’s at that moment with “BPM of the show”, else the cuelist’s own. They don’t follow the tapped rhythm.
- “Timecode”: the steps run from the first GO; they do not follow the MIDI timecode or the timeline of a super scene. The first step plays at the GO, not at the time of its cue. On a cue without a time, they stop until a GO.
- “Submaster”: the steps start with a GO, not with the fader.
- “Override”: the steps mix with what is below like any others.
- FX: only their base values.
- The delay, also that of an attribute or a fixture: the step starts at once.
- A down fade different from the fade: only approximate.
- The times per attribute and per fixture: only the fade of the cue.
- The fade curves: only linear.
- A Wait shorter than the fade, or in “Chase” and “Timecode” a step shorter than its fade: the next step waits for the fade to end.
- Links: the steps play in order. If the last cue links always to the first one, the steps start again.
- The actions of the cues: they do not run.
- In a Sequence, released values go to their rest value. In a Chaser, they are left to QLC+.
- With “Stay”, GO on the last cue stops the steps.
- GO on the beat and the tapped rhythm: the steps do not follow the beat.
- “Move in black”: the fixtures move as they fade up.
- “Released by another GO”: those lists go into a “Cuelists (one at a time)” frame, where starting one stops the others.

A cuelist that already has its cue list in the Virtual Console (a QLC+ show whose steps you converted to a cuelist) gets no other one: the old one plays it again. Try the copy in QLC+ before you use it live: QLC+ does its fades its own way.

### QLC+ cue lists

**“New” › “QLC+ cue lists…”** turns the chasers, sequences and Virtual Console cue lists of a QLC+ show into cuelists: from the open show or from another one (“From another show…”). Check the ones you want to convert. The Virtual Console cue lists come first, checked; then the chasers and sequences that no cue list uses. The ones that already have a cuelist with their name come unchecked.

Each step is a cue with what it changes; timed steps are chained with Wait and the ones that wait are GO. A Virtual Console cue list gives its name to the cuelist.

- **“Replace the originals”** (only in the open show): the cuelist takes their place and their ID. Their groups, SHOW buttons and mappings go to the cuelist, but QLC+ no longer sees them. Unchecked, the originals stay and the show still plays the same in QLC+.
- **“Put them on the free playbacks of this page”**. If they do not all fit, the message says how many are left out.
- **“Bring their keys and controller buttons as mappings”**: “Next”, “Previous”, “Playback” and “Stop” of each cue list become GO, Back, Pause/Back and Release of its cuelist. OSC controls, keys with Ctrl, Alt or Shift, inputs of a universe with no controller in this show and controls that already have a mapping do not come in. The message says how many mappings are made and which controls are left out. A button of an Akai APC comes with its LEDs, as when you learn it.

From another show only the cuelists come in, on the fixtures of this show that match theirs (the same address and model first). The message says how many fixtures are missing.

Not carried over: the random order (they play in order), beats (they become seconds at the current BPM), the side fader of the cue lists and steps that are neither scenes nor effects. Cue lists in the old QLC+ format cannot be converted: open and save them in QLC+ first.

When you open a QLC+ show with cue lists and no cuelists, the “Opened” message reminds you where to convert them.

## Folders

Folders organise the scene list (also in the SHOW, in super scenes, in racks and when you add scenes to a group).

- **Create**: the folder button with **+** next to the search box, or **“New folder…”** in the menu of a scene.
- **Put scenes in**: drag them onto the folder, or right click › **“Move to folder”**. It works with several selected scenes.
- **Take out**: right click › **“Remove from its folder”**.
- **Rename or delete**: the **⋯** button of the folder (or right click): **“Rename folder”** and **“Delete folder (its scenes stay)”**.
- Click the header of a folder to fold or unfold it.

Folders are saved with the show.

## Import and export

- **Export**: the export button in the header saves the scene to a `.dtcfx` file (“DMX Total Control effects”) to load it in other shows. It also works with racks.
- **Import**: **“New” › “Import from a file…”**. The fixtures are matched with the ones in this show; if one in the file is not here, its part is left out and you are warned.
- **The scenes of a whole other show**: in [Patch › Import](patch.md#importing-from-another-show), “Scenes of another show…”.
- **For QLC+**: “QLC+” in the header of a cuelist ([Cuelists and QLC+](escenas.md#cuelists-and-qlc)).
- **From QLC+**: “New” › “QLC+ cue lists…” ([QLC+ cue lists](escenas.md#qlc-cue-lists)).
