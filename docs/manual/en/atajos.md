# Keyboard shortcuts

All the keys and combinations of the program, by page. The keys you assign yourself in [Mappings](mapeos.md#keyboard) and the keys of the [SHOW](show.md#button-options) buttons are added to these. A key assigned in Mappings takes priority over the program shortcut that uses the same key.

## On any page

| Key | What it does |
|---|---|
| **F1** | Live |
| **F2** | Show |
| **F3** | Program |
| **F4** | Scenes |
| **F5** | Patch |
| **F6** | Fixtures |
| **F7** | Outputs |
| **F8** | Mappings |
| **F9** | Options |
| **F10** | Playbacks |
| **F11** | Full screen (and back to the window) |
| **F12** | Opens this manual on the page you are on (also the **“?”** button in the top bar) |
| **Ctrl+N** | New show |
| **Ctrl+O** | Open a show |
| **Ctrl+S** | Save the show (the first time, it asks for a name) |
| **Ctrl+Shift+S** | Save the show with another name (“Save as…”) |
| **Ctrl+B** | Blackout (on and off) |
| **Pause** twice (within 2 s) | Puts the [show stop](directo.md#show-stop) on or takes it away, on any page, with any modifier and even while you type. A single press only warns (“Show stop: press Pause again to put it”); holding it does nothing more. It can't be assigned in Mappings; a show that already mapped it keeps it |
| **Esc** | Stop all. In Learn mode, cancels the pending control or leaves the mode; with “Record” armed in Playbacks, disarms it; with a group isolated on the plan of Patch, ends the isolation (with a menu, a drop-down list or a window of Patch open, such as “Add fixtures”, “Edit” or “Limits”, it only closes it; a warning such as “Save changes?” doesn't close with Esc). In a window where you type a name or a number (“Create group”, “Rename”, “Renumber”…), closes it without applying anything: press Esc again to stop all |
| **Esc** twice | During the fade of “Stop all with a fade”, stops everything at once |
| **Ctrl+Z** | In Live and in the SHOW (not in its edit mode nor while you type), for 10 s after “Stop all” or “Release groups”: [bring back what was playing](directo.md#bring-back-what-was-playing) |
| **Ctrl+→** | Next scene in each playing group |
| **Ctrl+←** | Previous scene in each playing group |
| **G** | BeatGO (except on the Show page) |

If a page is locked, its key asks for the password (if there is one) and unlocks it. With the SHOW locked (kiosk mode) the page keys do not work, and with Setup locked neither do Ctrl+N, Ctrl+O and Ctrl+Shift+S.

T, G and the keys you assign in Mappings do not act while you are typing in a text field, such as the command line of Program. The Pause key (the show stop) does work while you type.

## Show mode

With [Show mode](directo.md#show-mode) on, when you close the program or open or start another show:

| Key | What it does |
|---|---|
| Hold **Space** or **Enter** for 1.5 s | Like holding the button: confirms (the key's auto-repeat doesn't count) |
| **Esc** | Cancels; in that window it never stops the show |

## Playbacks

| Key | What it does |
|---|---|
| **Space** | GO of the selected playback (one GO per press) |
| **Shift+Space** | Back on the selected playback |
| **PgUp** | Previous playback page |
| **PgDn** | Next playback page |
| **Esc** | Disarms “Record” |

They work on the Playbacks page as long as you are not typing in a text field.

## Program

| Key | What it does |
|---|---|
| Typing | What you type on the page goes to the command line |
| **Enter** | Runs the command |
| **↑** / **↓** | The last commands |
| **Esc** | Clears the command line (with the line empty, Stop all) |

See [the command line](programar.md#command-line).

## Show

| Key | What it does |
|---|---|
| The key of a button | Presses that button (while you hold it, if it is a flash button) |
| **Del** / **Backspace** | In edit mode, removes the control in the selected cell |
| **Esc** | In edit mode, clears the selection |

## Super scenes

| Key or gesture | What it does |
|---|---|
| **Space** | Play or stop from the cursor |
| **Del** / **Backspace** | Deletes the selected blocks |
| **Ctrl+A** | Selects every block on the tracks in view |
| **Ctrl+D** | Duplicates the selected blocks right after themselves |
| **Ctrl+C** | Copies the selected blocks |
| **Ctrl+X** | Cuts the selected blocks (those on locked tracks are only copied) |
| **Ctrl+V** | Pastes the copied blocks at the cursor, from the selected track |
| **Ctrl+Z** | Undoes the last change (not while you type in a field) |
| **Ctrl+Y** / **Ctrl+Shift+Z** | Redoes what was undone |
| **Ctrl+click** on a block | Adds it to the selection or takes it out |
| **Shift+click** on a block | Selects from the last block chosen to this one |
| Dragging on an empty part of the tracks | Draws a rectangle that selects the blocks it touches (with **Ctrl** or **Shift**, they are added) |
| Right click on an empty part of a track | “Paste here” and “Select all” |
| **Ctrl+wheel** | Zoom on the timeline |
| **Shift+wheel** | Move through time |
| Double click on the ruler | Play from there |

The block keys don't act while you type in a field. See [Selecting several blocks](escenas.md#selecting-several-blocks) and [Copy and paste](escenas.md#copy-and-paste).

## Faders and sliders

After you click a fader or a slider:

| Key or gesture | What it does |
|---|---|
| **↑** / **→** | Up one step |
| **↓** / **←** | Down one step |
| **PgUp** / **PgDn** | Up or down ten steps |
| Double click | Returns it to its rest value (usually zero) |

## Cuelists and steps (Scenes)

| Key or gesture | What it does |
|---|---|
| **Ctrl+click** / **Shift+click** | Selects several cues or steps to change their times at once |
| Double click on a cue or a step | Loads it into the programmer |
| Double click on the number of a cue | Changes its number |
| Wheel over a clicked time | ±0.1 s (with **Shift**, ±1 s) |
| **Ctrl+Z** | Undoes the last edit of the open cuelist (in a field with typed text, that text first) |
| **Ctrl+Y** / **Ctrl+Shift+Z** | Redoes what was undone |
| **Esc** in a field | Leaves the value as it was |

## Mouse and modifiers

| Gesture | Where | What it does |
|---|---|---|
| **Alt+click** | Almost any control | Takes it to the open SHOW page |
| **Shift+click** | Learn mode | Uses the control instead of mapping it |
| **Ctrl+click** | Flash buttons of the mixer | Keeps the flash on until the next click |
| **Ctrl+click** / **Shift+click** | Fixtures, groups and 2D plan (Program and Patch), scene list, blocks of a super scene | Adds to the selection or removes from it |
| Wheel | 2D plan | Zoom |
| Drag with the right or middle button | 2D plan | Moves the view (double click: centres it) |
| Right click | 15° button of the 2D plan | Rotates the other way |
| Right click | SHOW button (live) | Stops what it plays (its scene, its group, its playback or its palette), also with GO; an “Arm effects” button disarms them |
| Hold down “Unlock” | Locked Show | Unlocks |
| Hold down for 1 s | “Show stop” button of the SHOW, a mapped pad or key, the hand of the phone, “Hold to release” | Puts the show stop on or takes it away |
