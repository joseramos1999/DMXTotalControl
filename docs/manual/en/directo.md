# Live

![The Live page](../img/directo.png)

**Live** (F1) is the page to play scenes during the show. Scenes are arranged in **groups**, which are columns that work in parallel: each group has its scenes, its mode (exclusive or mix), its crossfade time and its master.

In the page header you have:

- **◀ ▶**: previous or next scene in each playing group (also with Ctrl+← and Ctrl+→).
- **“Pause”** / **“Resume”**: freezes everything that is playing and continues it.
- **“Release groups”**: stops the scenes of all the groups.
- **“Fold groups”** / **“Unfold groups”**: folds every column at once, to see them all; when they are all folded, it unfolds them.
- **“Mixer”**: switches to the [mixer](directo.md#mixer) view. An orange dot warns you that the mixer is changing the output.
- **“New group”**: adds a column.

## Groups

### Creating and arranging groups

1. Press **“New group”** (or the “Add group” box at the end of the columns).
2. Click the name of the column to change it (for example `Wash`, `Spots`, `Strobes`, `Effects`) and press Enter.

The gear button of each column opens its menu: **“Change colour”** (goes to the next colour), **“Move left”**, **“Move right”** and **“Delete group”**. You can also drag the group by the handle to the left of its name and drop it on another group to take its place.

The **‹** arrow next to the name **folds** the group: it becomes a narrow strip with its name and how many scenes are playing, to make room for the others. Its scenes go on playing. The **›** arrow on the strip unfolds it. It is saved with the show. To fold or unfold them all at once, use **“Fold groups”** / **“Unfold groups”** in the header.

### Adding and ordering scenes

1. Press **“Add scenes”** at the end of the column.
2. In the list, click the scenes you want (they are marked with ✓). You can search by name or by folder and view them by folders or by groups.
3. Press **“Add N”**. Double-clicking a scene adds it straight away.

To reorder, hover over a tile and drag it by its handle (on the right of the tile): drop it on another scene to put it in front of it, or in the empty space of the column to put it at the end. You can drop it in **another group** and it moves with its options. In the tile menu (right click) you also have **“Move up”**, **“Move down”** and **“Remove from group”**.

> **Tip:** when you record from Program you can put the scene straight into a group with “Add to group”. See [Recording](programar.md#recording).

## Playing scenes

Click a tile to play the scene (with **GO**, the mode the program starts in). To release it, press the square in the header of the group (“Release this group”) or switch to **Stop** and press it. The playing tile takes the colour of the group and has a pulsing dot.

Each tile shows:

- The **name** and, below it, the scene **type**. On a playing cuelist, the cue it is on (“Q 3 · Verse”). After that, the [tapped rhythm](directo.md#rhythm-of-a-scene-with-steps) if it has one (“120 BPM”).
- **P7**, **P2**…: the priority, only when it is not the normal one (5). In orange if it is higher than 5.
- A **lightning bolt**: it is a flash scene.
- A **bar split in two** at the bottom: a playing scene with steps, cuelist or FX at “BPM of the show”; it alternates on each step (on an FX, on each beat), at its rhythm.

### Exclusive or mix

The button of each column switches between:

- **“Exclusive”**: only one scene plays at a time. When you play another one, it crossfades to the new one. The [SHOW faders](show.md#faders) that play scenes are the exception: they add theirs, so you can raise several of the same group at once.
- **“Mix”**: scenes add up; you can have several playing at the same time.

### Crossfade time

The drop-down next to “Exclusive”/“Mix” is the fade time of the group:

- **“By scene”**: each scene uses its own fade in and fade out.
- **“Snap”**: no fade.
- **0.5 s to 10 s**: that time for all the scenes of the group, instead of their own.
- An FX scene that goes out keeps moving while it fades to what is below, with the crossfade of the group or with its own fade out (also with “Stop all with a fade”).

### Previous and next scene

The **◀ ▶** buttons of each column play the previous or next scene of the group, in the order of the column. The ones in the page header do the same in all the groups that are playing.

### Flash scenes

A **flash** scene plays only while you hold its tile down. You turn this on in its [properties](directo.md#scene-properties) with “Flash (plays while pressed)”.

### Cuelists in a group

A cuelist in a group is just another tile: a click plays it (with **Stop**, it stops it). Its menu (right click) adds **“GO (next cue)”**, **“Back (cue before)”** and **“Release the cuelist”**. To run it comfortably with GO, put it on a [playback](playbacks.md).

### Rhythm of a scene with steps

Scenes with steps (sequences and chasers), cuelists and FX scenes with FX at “BPM of the show” follow the rhythm at which you press their tile:

- One click plays it (with **GO**, the mode the program starts in; see below).
- With the scene playing, press its tile **several times in a row** to the beat of the music, with less than 2 seconds between presses. Each press goes to the next step (on a cuelist, it gives a GO) and the scene carries on by itself at that rhythm: one step on each beat, instead of its own times. The rhythm is the average of the last presses. A press far off that rhythm (on the next bar, to put it back in time, or after a beat you missed) is a beat that does not change it; two such presses in a row that agree tap a new rhythm. A press that comes a little late, just after the step of that beat (less than half a beat), is that same step: it gives no other one, it only puts the rhythm back in time.
- Only the presses made while the scene is playing count: the one that plays it starts the count. So switching scenes in an exclusive group and coming back does not give a rhythm to either of them.
- After a pause of more than 2 seconds, a click gives one step without changing its rhythm, and the scene keeps playing. To stop it, switch to **Stop** and press it.
- The rhythm you tapped is played, also with a **live speed** set; if you then move the speed, the rhythm speeds up or slows down with it. While paused, presses do not give steps.
- The rhythm is stored with the scene and belongs to it: it also follows it when it plays in a super scene or on a playback. In its [properties](directo.md#scene-properties) (and in [Scenes](escenas.md#how-a-scene-plays)) you can type it or press **“Clear”** to go back to its own times.
- In an **FX scene**, the rhythm is that of its FX at **“BPM of the show”**: the first press of a row starts their cycle again (as when it is played), and each next one takes them to the nearest beat of their clock (a chase goes on to the next fixture; at ×¼, they go a quarter of a cycle further; a press a bar later, to the beat they are on). Several in a row give them one cycle on each beat (times their multiplier), instead of the BPM of the show. The FX with a speed of their own do not change. With no tapped rhythm, or after “Clear”, they follow the BPM and the beat of the show: the Tap of the show puts them back on it, and a press of their tile takes them off it until then. If you turn off “BPM of the show” on all its FX, the scene loses its tapped rhythm.

It works the same with its [SHOW](show.md) buttons, with its controller and key mappings in “Toggle” mode and with the [remote control](remoto.md).

#### GO and Stop

The **GO** and **Stop** buttons of the top bar (and of the “Rhythm: GO and Stop” section of the SHOW) decide what pressing any scene does, for everyone who presses it. One of the two is always on: the program always starts in **GO**, and pressing the other one switches the mode:

- **GO** on: pressing a scene **plays** it; on one that is already playing, each press gives a step (on a cuelist, a GO; on an FX scene, the first one starts the cycle of its FX at “BPM of the show” again and the next ones take them to the next beat) and several in a row tap its rhythm. It never stops it, even if you go more than 2 seconds without pressing: this way you can tap the tempo calmly.
- **Stop** on: pressing a scene **stops** it. One that is not playing stays as it is.
- To release the scenes of a group without changing the mode, press the square in its header (“Release this group”); **“Stop all”** (Esc) stops everything.

Flash scenes keep working while you hold them down, in any mode.

While it plays, each tile of a scene with steps, of a cuelist or of an FX scene with FX at “BPM of the show” has a **rhythm bar** split in two at the bottom: one side lights up and then the other on each step (on a cuelist, on each cue; on an FX scene, on each beat, a cycle of those FX at ×1). This way you see the rhythm of each one. Its SHOW buttons, its playback strips and its buttons on the phone have it too.

## Masters

- **Group master**: the “Master” slider at the foot of each column scales the intensity of all its scenes.
- **Grand master**: the “GM” in the top bar scales the light of the whole output: the dimmers and, on fixtures without a dimmer, their colours. It leaves the colour mixing alone (cyan, magenta, yellow, hue and saturation), and the colours of a fixture or head with a dimmer of its own, which go down with that dimmer. In Options you decide whether it acts only on the intensity or on all channels. It doesn't lower the fixtures **protected from the grand master** (in the [fixture limits](patch.md#limits) or its menu): the house lights, a media server, a hazer. If there are any, an **anchor** next to “GM” says so (when the bar has room for it) and, on hover, how many (“The grand master doesn't lower 3 fixtures protected from it”).
- **Live dimmer**: each scene also has its own dimmer, in its properties, on top of the group master.

The masters and the grand master do not affect the channels that have the “Dimmer” option turned off in the [fixture limits](patch.md#limits). If it is the dimmer of a fixture, it no longer counts as a dimmer of its own: the grand master then lowers its colours.

## Scene properties

Right click a tile and choose **“Properties…”**.

| Field | What it does |
|---|---|
| **Name** | The name of the scene. |
| **Fade in** / **Fade out** | Its times, in seconds. If the group has a crossfade time set, that one is used. |
| **Priority (0-10)** | Which scene wins when two touch the same channel (see [Priorities](directo.md#priorities)). |
| **When it starts, release** | What it stops when it starts: [release modes](directo.md#release-and-protection-modes). |
| **Protected from releases by** | Which releases it ignores. |
| **Flash (plays while pressed)** | Turns it into a flash scene. |
| **Live dimmer** | Its intensity, on top of the group master. |
| **Tapped rhythm** | Steps, cuelists and FX scenes with FX at “BPM of the show”: the [rhythm tapped](directo.md#rhythm-of-a-scene-with-steps) with its tile, in BPM. Type it or press “Clear” to go back to its own times (on an FX scene, to the BPM of the show). It acts at once. |
| **Live speed** | From ×0.1 to ×10 (scenes that move: steps, FX, cuelists…). |
| **Live phase** / **Live size** | The phase (0–359°) and the size (0–200 %) of the FX. |
| **Fade speed** | Speeds up or slows down its fades, from ×0.1 to ×10. |

The live sliders act at once, while you move them, and change how the scene plays in that group, not the recorded scene. The other fields are applied when you press **“Apply”**.

The same tile menu has **“Open in Scenes”**, **“Edit in the programmer”** (static scenes) and the **“Priority: N”** submenu to change it without opening the properties.

## Priorities

Each scene has a priority from 0 to 10; new ones take the one you set in [Options](opciones.md#playback) (5 by default).

- The scene with the **highest priority rules on every channel it touches**, zeros included: a red scene with priority 8 over a blue one with priority 5 gives red, not magenta.
- With the **same priority**, the intensity channels (dimmer, RGB…) are mixed and the highest value wins; on the other channels the **last change** wins: the last scene played, or the last GO of a cuelist that changed that channel. What a cuelist only keeps from earlier cues is not taken back.
- If you change the priority of a playing scene, it applies at once.
- When you release a higher priority scene with a fade, it fades to what is underneath.
- **The programmer overrides everything**, intensities included. In Options you can lower the priority of the live edits.

## Release and protection modes

**When it starts, release** decides which scenes stop when this one starts:

- **“Group default”**: in an exclusive group, the others in its group; in a mix group, none.
- **“None”**: stops nothing.
- **“Its group”**: the other scenes of its group.
- **“All the scenes”**: all the others.
- **“The other groups”**: the scenes of the other groups.

When a [SHOW fader](show.md#faders) plays it, it releases nothing, whatever its mode: it adds itself. With its button, it releases as usual. And while that fader is up, nothing stops it but lowering it all the way.

**Protected from releases by** makes the scene ignore those releases:

- **“None”**: it is released as usual.
- **“Everything (manual stop only)”**: only you can stop it.
- **“Its own group”**: ignores the ones from its group.
- **“The other groups”**: ignores the ones that come from other groups.

> **Tip:** give an ambient scene the protection “Everything (manual stop only)” and play scenes on top of it with “All the scenes”: the ambient stays.

## Pause, release and stop

- **“Pause”** freezes everything that is playing. **“Resume”** continues it.
- **“Release groups”** stops the scenes of all the groups; the square of each column, only those of that group. For 10 s, [“Bring back what was playing”](directo.md#bring-back-what-was-playing) starts again what “Release groups” stopped (its button, or a pad or a SHOW button in “All groups”); what is released in a single group (its square, or a pad or a SHOW button of that group) or by an action of a cue can't be brought back.
- **“Stop all”** (Esc), in the top bar, stops everything that is playing, also outside the groups. It does not clear the programmer. Neither “Stop all” nor “Release groups” stops the scenes a raised [SHOW fader](show.md#faders) holds: only lowering it all the way. It also disarms the [special effects](directo.md#protected-special-effects), and for 10 s you can [bring back what was playing](directo.md#bring-back-what-was-playing). With [“Stop all with a fade”](directo.md#stop-all-with-a-fade), the functions fade out before they stop.
- **“Blackout”** (Ctrl+B) sends the output to black without stopping anything. The heads don't move: pan, tilt, zoom, focus and the other channels that aren't light stay where they are. It also disarms the special effects. To take the light down slowly and end in Blackout, use a [“Fade to black”](directo.md#fade-to-black).

## Safety during the show

So that a mistake or a failure of the computer doesn't spoil the show: [Show mode](directo.md#show-mode), [protected special effects](directo.md#protected-special-effects), [parked channels](directo.md#parked-channels), the [show stop](directo.md#show-stop), [bring back what was playing](directo.md#bring-back-what-was-playing), [stop all with a fade](directo.md#stop-all-with-a-fade) and [fade to black](directo.md#fade-to-black). Their settings are in [Options › Safety during the show](opciones.md#safety-during-the-show).

> **Important:** the computer must never be the only safety of a CO2, flame, cold spark or confetti machine. Set the machines and the DMX nodes to switch off when they lose the DMX signal (not to hold the last value), fit a hardware emergency stop or dead-man switch where the machine needs one, and follow the rules of the venue and the instructions of the manufacturer. If the program hangs or the computer goes off, no software can cut a fire: only the equipment can.

### Show mode

The **shield** in the top bar, right after the padlock, turns **Show mode** on or off in one click. When it is on it is orange, and the second line of the title starts with “Show mode ·”. Locking the SHOW (also at startup with “…and locked”) turns it on by itself; unlocking doesn't turn it off. With the SHOW locked, the shield shows but can't be clicked. It is a setting of the computer, not of the show.

With Show mode on, these actions ask you to hold a button for 1.5 s (“Hold to close”, “Hold to open” or “Hold to continue”), even with no unsaved changes:

- closing the program (the × of the window, Alt+F4, the taskbar) and installing an update;
- **New** and **Open** (also Ctrl+N and Ctrl+O), the recent shows, the examples and the practice show of the guide;
- opening a show from Windows Explorer.

Letting go too early, **“Cancel”** or Esc cancel; Esc in that window never stops the show. Holding Space or Enter works too (the key's auto-repeat doesn't count). Then, if there are unsaved changes, “Save changes?” comes as always. When Windows shuts down or you sign out with Show mode on, the question may hold up the shutdown: close the program first.

In Show mode, if the laptop goes onto its battery, a red bar “Running on battery: plug in the charger” shows on every page (also locked and in full screen) until the charger is back; at 20 % or less it adds “Battery at N %”. The battery is checked every 30 s and on each power message of Windows. Without Show mode, a message says it once (and again when it gets low), and [Options](opciones.md#safety-during-the-show) shows it.

If the Windows power plan has USB selective suspend on, at startup a blue bar warns that the DMX interface may stop, with **“How to turn it off”** (it opens [USB selective suspend](opciones.md#usb-selective-suspend)) and a × that hides it until the next start. The program only reads these Windows settings: it never changes them.

### Protected special effects

A **protected special effect** is a fixture that can't fire by mistake. CO2, flame, cold spark and confetti machines are protected by themselves, and any fixture can be protected by hand (see [Patch › Protected special effects](patch.md#protected-special-effects)). They carry a shield in the list and on the plan of the Patch.

- **Disarmed** (the program always starts so), all their channels send 0, whatever plays.
- **Armed**, they only go out from **scenes** (flash ones too), **cuelists**, **chasers**, **sequences**, **super scenes** and the base values of **FX scenes**. So you fire them with a SHOW button in flash (it fires while you hold it), a Live tile, an APC pad, a playback or the [phone](remoto.md#safety-on-the-phone).
- **Never**, not even armed, do these fire them: the programmer, Identify, Highlight, the mixer (Full, Flash, colour, strobe), the FX and their macros, Move FX, Colour FX and plan mappings, Curve FX, scripts or a DMX input passed through to the output. The grand master doesn't scale them.
- The [limits](patch.md#limits) only count while they fire: at rest 0 always goes out. Don't invert the channels of an effect machine: an inverted fire channel only fires with a value that, inverted, turns it on.
- Their pan and tilt channels (those of a flame head) only go out armed and from those functions, with no maximum time; disarmed, at 0. All the others but the safety one fire, and the maximum time applies to them (see [Patch › Protected special effects](patch.md#protected-special-effects)).

#### Arm effects

The **“Arm effects”** button (with a flame) is in the top bar, next to “Blackout”, only when the show has protected effects, also with the SHOW locked. A click arms them and another disarms them. Armed, the button turns red with the time left before they disarm by themselves (“ARMED · 9:59”), and its border pulses while one fires. In a narrower window it says “Arm” or shows only the flame: below 1450 px wide, “Arm effects”, “Blackout” and “Stop all” keep only their icon, and below about 1200 px the running count hides and the grand master gets shorter.

Armed, a big red bar **“SPECIAL EFFECTS ARMED”** shows on every page (also locked and in full screen) with what fires (“Firing: CO2 1”) or what waits for a release, and a **“Disarm”** button.

The **▾** arrow next to it shows whether they are armed and how long until they disarm by themselves, **“Firing now”**, **“Waiting for a release”** (let go of its scene, its button or its fader, to 0, and it can fire), **“Last fires”** (name · time · seconds · how it ended: released, cut at its maximum time, disarmed…), **“Open the log”** and **“Each effect: Patch”** (these two, not with the SHOW locked).

They don't arm if no fixture is protected, with Blackout on (“Blackout is on: turn it off to arm the special effects”) or with the show stop on. You can also arm them with a [SHOW button](show.md#button-actions), a [mapping](mapeos.md#drives) or the [phone](remoto.md#safety-on-the-phone).

#### Maximum time and minimum wait

- A fire lasts while its scene, its button or its fader gives it, at most its **maximum time** (by default: CO2 3 s, flames 2 s, sparks 5 s, confetti 2 s). Then it is cut, even if you keep pressing, and it doesn't come back until you let go.
- Between one fire and the next there is a **minimum wait** (1 s by default) from the release. A press during the wait doesn't fire later: let go and press again.
- Arming while a scene or a fader already gives the effect a value doesn't fire it: it waits for you to let go. Disarming while one fires cuts it, even if you arm again at once (it doesn't come back until you let go); a control that only arms does nothing when they are already armed.
- The **safety channel**, if the machine has one, only goes out within its values while the effect is armed and firing; otherwise 0. On its own it never fires the machine.
- If the program stops sending frames (it is very busy), each universe still cuts a fire at its time, at most 40 ms late.

#### When they disarm

They disarm by themselves:

- with **Stop all** (Esc, its button, a pad, a SHOW button or the phone) and with **Blackout** (Ctrl+B, its button, a SHOW button, a pad or the phone; not the “Blackout” of a mixer strip);
- with the **show stop**, a **fade to black** (when it starts) and **bring back what was playing**;
- when you open or start **another show**, and when you **close** the program;
- after some **minutes without a fire** (10 by default: “Disarm the special effects unused after”, in [Options](opciones.md#safety-during-the-show)), never while one fires;
- when the last protected effect is removed or unprotected, and **by hand**.

A message says it (“Special effects disarmed: Stop all”). They are never saved armed. A SHOW fader that holds a scene with a CO2 keeps holding it after Esc, but the CO2 stays at 0 until you take the fader all the way down.

#### The fire log

Each session writes a file in `%LOCALAPPDATA%\DMXTotalControl\DMX Total Control\effects-log` (“Open the log” opens the folder): each fire with its time, the number and the name of the fixture, how long it lasted and how it ended (released, cut at its maximum time, disarmed, show stop, parked, removed), and when the effects were armed and disarmed, and why.

> **Not tested with real equipment:** none of this has been tested with CO2, flame, spark or confetti machines, with their safety channels, with Art-Net or sACN nodes, or with what they do when they lose the DMX signal. Try it with your equipment, with no audience and the machine not loaded, before you use it in a show.

### Parked channels

A **parked** channel always goes out at a fixed value, above the scenes, the programmer, the mixer, the grand master and Stop all: the hazer at 30 % all night, a broken head off and still. Esc and Blackout never unpark it; with Blackout its intensities go dark, unless it has “Also in blackout”. You park in the [Patch](patch.md#parking-channels) and in [Program](programar.md#parking). When a show with parked channels opens they apply at once, and a message says which (“2 parked fixtures: HAZER, Spot 3 (Patch)”).

### Show stop

The **show stop** is a safety light to stop the show and give people light: above everything, it puts open white on the lights, darkens strobes and lasers, keeps the heads still and disarms the special effects.

To **put it or take it away**, press the **Pause key twice** (within 2 s; not in a show that maps it, see [Mappings](mapeos.md#keyboard)), or **hold for 1 s** a SHOW button “Show stop (hold 1 s)”, a [mapped](mapeos.md#drives) pad or key, or the hand in the bar of the [phone](remoto.md#safety-on-the-phone). A shorter press does nothing, and a single press of the Pause key only warns (“Show stop: press Pause again to put it”). While you hold, an orange bar “Show stop: keep holding…” fills. Once it is on, a big red bar **“SHOW STOP”** shows on every page (also locked and in full screen) with **“Hold to release”** (hold it a second). If the program loses the keyboard while you hold a mapped key (another window, a Windows message), the hold is let go and nothing changes.

While it is on:

- the special effects are disarmed (“…disarmed: show stop”) and can't be armed (“The show stop is on: take it away to arm the special effects”);
- the lights (moving heads, scanners, PARs and washes, LED bars, blinders and dimmers) of the chosen groups go to **open white** at the chosen level: the dimmer at the level, red, green, blue and white at full (without a dimmer, the colours at the level), CMY, amber and UV at 0, the shutter open, the colour wheel at white or open and the built-in programs at “no function”;
- strobes go dark (their intensities at 0, or all their channels if they have none) and lasers go to 0 on all their channels (a DMX laser is off at 0: check yours);
- every shutter opens (no strobe; a “slow to fast” strobe channel goes to 0) and the mixer strobe stops;
- moving heads stay where they are (pan and tilt, the fine ones too);
- smoke, hazers, fans, bubbles, snow, other effects, gobos, prisms and zoom stay as they are, and protected effects at 0.

It wins over the programmer, the grand master, Blackout (the rest stays black) and parked channels: a fixture parked off lights up too, and a parked hazer stays parked. The [limits](patch.md#limits) still count (an inverted dimmer, a maximum). Which fixtures it lights and at what level is chosen in [Options › Show stop](opciones.md#show-stop).

- **Stop all** (Esc) during the show stop stops the functions underneath, and the show stop stays.
- When you **take it away**, the show is as its functions play now (they kept playing underneath) and the mixer strobe comes back. The effects stay disarmed: arm them by hand; a held CO2 has to be let go first.
- **New**, **Open** and closing the program take it away. It is never saved on: after an unexpected close or a restart, put it again.
- The effects log of the session writes “Show stop on” and “Show stop off”.

> **Not tested with real equipment:** the open white and shutter values of real fixtures, a laser really dark at 0, the Pause key of real keyboards, the APC LEDs and the phone over a real Wi-Fi.

### Bring back what was playing

After **Stop all** (Esc, its button, a pad, a SHOW button or the phone) or **“Release groups”** (all the groups, not a single one), a blue bar under the top bar says what was stopped (“Stopped: 4 scenes and 1 cuelist · Bring back what was playing (Ctrl+Z)”; “Released: …” after “Release groups”), with the **“Bring back”** button, a strip that runs down in 10 seconds and a × that hides it until the next Stop all. It shows with the SHOW locked too. **Ctrl+Z** does the same in Live and in the SHOW (not while you edit it or type), and the bar only says “(Ctrl+Z)” where it works. “Bring back” ignores a tap in its first half second (a second panic tap on Stop all).

These come back, with a 0.5 s fade:

- the scene each group played;
- the scenes started from SHOW buttons, from Scenes or from playbacks;
- the cuelists, on the cue they were on, at the level of their playback; a paused cuelist comes back paused.

FX and steps start their cycle again, functions in beats use their own fade and “Timecode” cuelists follow their timecode. These don't come back: flashes that were held (in Live, the SHOW, pads or playbacks), the scenes a SHOW fader holds (they never stopped), the programmer, the mixer, the grand master, the arming of the effects, parked channels and the show stop.

It works once, and a message says what came back (“Brought back: 4 scenes and 1 cuelist”). Another Stop all within the 10 s joins the first (not the second Esc during the fade of “Stop all with a fade”); “Release groups” keeps only the groups, and a Stop all after it joins in. Functions deleted in the meantime are skipped, and a new or opened show clears it.

Bringing back **never arms** the special effects: if they were armed (after “Release groups”, which doesn't disarm, or when you armed them again after Stop all), it disarms them first (“Special effects disarmed: Bring back what was playing”). What comes back with a CO2 waits for you to let go and stays at 0, also after you arm again.

### Stop all with a fade

With **“Stop all with a fade”** in [Options](opciones.md#safety-during-the-show) (from 0 to 5 s; 0, at once, as always), Stop all fades the functions out over that time. The scenes SHOW faders hold and parked channels stay; protected effects go dark at once (they are disarmed). While it fades, the “Stop all” button has a strip and says “Fading out: Esc again stops at once”: **Esc again** stops everything at once, also what you started during the fade. Afterwards you can bring back what the first Esc stopped; what you started during the fade doesn't come back.

### Fade to black

A **“Fade to black”** (a [SHOW button](show.md#button-actions) or a [mapping](mapeos.md#drives), with its time: 3 s by default) takes the intensities down at the output over that time and then puts Blackout on, for example at the end of a set. Underneath, the light is at full again: when you turn Blackout off, it comes back.

- It acts above the grand master (also on the fixtures protected from it) and on parked channels without “Also in blackout”.
- It doesn't touch parked channels with “Also in blackout”, pan, tilt or other channels that aren't light, nor protected effects.
- When it starts, it disarms the special effects.
- Another press, or Blackout, end it at once. With Blackout already on, it does nothing.
- While it fades, the “Blackout” button has a strip and says “Fading to black: Blackout comes at its end (Blackout now: at once)”.

> **Not tested with real equipment:** Show mode (the battery, keeping the computer awake, USB suspend), bringing back, Stop all with a fade and Fade to black have not been tested with real laptops, DMX interfaces, APC LEDs or effect machines.

## Mixer

The mixer acts on the output, on top of every scene and of the live edits. Press **“Mixer”** to see it: there is a strip for **“All fixtures”** and one for each [fixture group](patch.md#fixture-groups).

Each strip has:

- A **dimmer fader** with its percentage.
- **“Full”**: the group at full. **“Blackout”**: the group to black, but for protected special effects (no button of the mixer turns them off or disarms them).
- **“Hue”** and **“Saturation”**: tint the colour. **“Reset colour”** removes the tint.
- **“Strobe”** and its speed, from 0.5 to 20 Hz.
- **“Flash”**: at full while pressed. **“Flash excl.”**: that group at full and the rest to black while pressed.
- **“Full excl.”**: that group at full and the rest to black.

Ctrl+click on a flash button keeps it on until the next click. **“Reset the mixer”** returns everything to its normal state. The border of a strip turns orange with a flash or a full, and red with blackout.

> **Tip:** the mixer can also be a section of the [SHOW](show.md), and its faders and buttons can be [mapped](mapeos.md) to a MIDI controller.

## Controls in the SHOW and on controllers

The scene tiles, the group masters, the ◀ ▶ and release buttons of each group, the ones in the header (◀ ▶, “Pause” and “Release groups”) and the mixer controls:

- With **Alt+click** they are copied to the SHOW page that is open (see [Show](show.md#bringing-any-control-to-the-show)).
- In Learn mode they can be assigned to a controller (see [Mappings](mapeos.md#learn-mode)).

While a mapping uses “The selected scene”, or in Learn mode, the tile of the selected scene (the last one you pressed) has a thin bar along its bottom edge (see [Mappings](mapeos.md#the-selected-scene)).
