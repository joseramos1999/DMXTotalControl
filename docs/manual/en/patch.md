# Patch

![The Patch page](../img/patch.png)

**Patch** (F5) holds the fixtures of your rig: what they are, which universe and address they are on, their number, the fixture groups and where they are on stage.

In the header you choose the view and you have the tools to add things:

- **“List”**: all the fixtures by address.
- **“Channels”**: the 512 channels of each universe, with the fixtures on top.
- **“2D plan”**: the stage seen from above.
- **“Import”**, **“LED matrix”** and **“Add fixtures”**.
- With parked fixtures, **“N parked · Unpark all”**: how many there are and the button to unpark them all (see [Parking channels](patch.md#parking-channels)).

## Adding fixtures

1. Press **“Add fixtures”**.
2. Choose the **Manufacturer** (type in “Search manufacturer…” to filter) and the **Model**.
3. In the **Patch** panel:
   - **Name**: the one you will see throughout the program (by default, the name of the model).
   - **Mode**: the DMX mode, with its number of channels.
   - **Quantity**: how many of the same.
   - **Universe** and **Address**: the address is set automatically to the first free one in the universe.
   - **Gap**: free channels between one fixture and the next.
   - With more than one, **“Make a group with them”** and the name of the **Group**.
4. Below, you see which addresses they will use (“Uses addresses 1 to 20”). If they do not fit in what is left of the universe, they go on to the next one. If the mode has more *DMX breaks* (a GDTF fixture with two DMX inputs), a note asks you to patch its “(DMX break 2)” mode too, at its own address.
5. Press **“Patch fixture”** or **“Patch N fixtures”**.

The window stays open so you can keep adding, with the address already at the next free one. If something cannot be patched (the addresses are taken, for example), the reason appears in red at the bottom. Press **“Close”** when you finish.

For single dimmers, choose **Generic › Generic Dimmer**: instead of a mode, you ask for a number of **Channels**.

**“Search GDTF-Share…”**, at the bottom, opens [GDTF-Share](fixtures.md#gdtf-share): download the fixture and press **“Patch it”** to have it chosen here.

> **Tip:** if you cannot find your fixture, search GDTF-Share, download the [Open Fixture Library](fixtures.md#open-fixture-library), import its GDTF or create it on the [Fixtures](fixtures.md) page.

## The list

Columns: selection checkbox, **#** (number), **NAME**, **ADDRESS** (universe . address), **CH**, **FIXTURE** (manufacturer and model), **MODE**, **INVERT** (pan and tilt) and the identify button. After the name, a **shield** marks a [protected special effect](patch.md#protected-special-effects) and a **padlock** a fixture with [parked](patch.md#parking-channels) channels.

- **“Group by fixture”** groups them by model, with a checkbox to select all the fixtures of each model. A click on the header of a model folds it (hides its fixtures) or unfolds it.
- A **double click** on a fixture edits it.

## Selecting

The tools of the bar work on the selection:

- A **click** selects; **Ctrl+click** adds or removes; **Shift+click** selects all the fixtures from the last one you clicked to that one (in that direction, which is the order of the selection), and **Ctrl+Shift+click** adds them. **“All”** and **“None”**. The checkbox of a row adds or removes its fixture (like Ctrl+click); with Shift, it adds all of them from the last one you clicked.
- **“Identify”**: the selected fixtures blink in white so you can find them on stage. **“Stop identifying”** stops it (it also stops when you leave the page). The button on each row identifies only that fixture. The list stays where it was. Protected special effects are never identified (blinking a CO2 would fire it): a message says how many were left out.
- **Invert pan** and **invert tilt** (the arrow icons): for the fixtures that have them.
- **“Limits”**, the **flame** button (**“Special effect”**), **“Edit”**, **“Re-address”**, **“Renumber”**, **“Create group”** and **“Delete”**: explained below.

A **right click** on a row of the list, or on a fixture of the 2D plan without moving the mouse, opens the fixture menu: **“Special effect…”**, **“Identify”**, **“Limits…”**, **“Edit…”**, **“Park as it is now”**, **“Park off”**, **“Also in blackout”**, **“Unpark”** and **“Unpark all”**. If the fixture wasn't selected, it selects it first.

## Addresses and universes

- **Editing a fixture**: select one and press **“Edit”** (or double-click it). You can change **Name**, **Mode**, **Universe**, **Address** and **Number**, and press **“Apply”**.
- **Re-addressing several**: select them and press **“Re-address”**. Choose **Universe**, **Start address** and **Gap**: they get consecutive addresses, in the order of the list.
- **Dragging in the Channels view**: choose the universe at the top and drag a fixture to another address. The shadow shows in green if it fits and in red if the channels are not free. At the top you see how many of the 512 channels are in use. A double click on a fixture edits it.

Universes are created and connected to the interfaces in [Outputs](salidas.md).

## Fixture numbers

Each fixture has a **number**, which is the one the [command line](programar.md#command-line) of Program uses (`1 THRU 8 @ FULL`). A new fixture gets the number after the last one.

- Change one in **“Edit”** › **Number**.
- To number several in a row, select them, press **“Renumber”** and type the **First number**: they are numbered in address order.

## Fixture groups

Fixture groups let you select with one click in Patch and in Program (`GROUP 2` in the command line), and each one has its own strip in the [mixer](directo.md#mixer). They are also used in the SHOW (colour wheel, XY grid, palettes, dimmer and movement speed) and in the mappings.

1. Select the fixtures.
2. Press **“Create group”** and type its name (for example, `Front wash` or `Spots`).

The **“Fixture groups”** panel on the left shows them with their number of fixtures. A click on a group selects its fixtures (in the “2D plan” view, it isolates it: see [Isolating a group on the plan](patch.md#isolating-a-group-on-the-plan)); **Ctrl+click** or **Shift+click** adds them. Right click:

- **“Rename”**.
- **“Move up”** and **“Move down”**: move it in the list.
- **“Replace with selection”**: the group now has the selected fixtures.
- **“Save the selection here…”**: see [Selections](patch.md#selections).
- Selection patterns, in the group order: **“Select odd”**, **“Select even”**, **“Select in reverse order”**, **“Select the first half”**, **“Select the second half”**, **“Select from the ends to the middle”** and **“Select from the middle to the ends”**.
- **“Delete group”**.

**The order of the groups** is yours to choose: drag a group by its grip (left of its name) to another place in the list, or use “Move up” and “Move down”. **“A–Z”**, in the header of the panel, orders them alphabetically (numbers as numbers: “Truss 2” before “Truss 10”). New groups go at the end. The order is saved with the show, and it is the same in Program and in the `GROUP` numbers of the command line.

### Selections

A **selection** is an ordered part of a group: for example, the spots of a group of six taken from the ends to the middle. The order matters, because fans and FX follow it.

1. Select the fixtures in the order you want (or with a pattern from the group menu).
2. Press the **+** of the group (or “Save the selection here…”) and type a name.

Selections appear under their group. A click selects their fixtures in their order, here and in Program. Right click: **“Rename”** and **“Delete selection”**. The arrow to the left of the group hides or shows its selections.

## 2D plan

The plan shows the stage seen from above, with the **AUDIENCE** at the bottom. You use it to select in Program, to see the output and for the colour effects laid out on the plan.

In the plan header:

- **“Width (m)”** and **“Depth (m)”**: the size of the stage, up to 200 m. They grow or shrink it on the right and at the front; a fixture left outside a smaller stage stays where it is.
- **“Auto arrange”**: spreads out all the fixtures.

Outside the stage, in the middle of each side, there is a **“−”** and a **“+”**: they remove or add 1 m on that side (on the left, on the right, at the back or at the front, on the audience side); hold them down for several metres. The fixtures stay where they are on the stage: on the left and at the back they move with that side, and so does the point [Follow](programar.md#follow-aiming-at-a-point) aims at, and the new strip appears on that side. While the mouse is over them, the view of the plan doesn't change and the buttons don't move, so you can click several times in a row; when you move it away, the plan fits again (if you had zoomed or moved the view, it stays as it is). The **“−”** of a side only works when no fixture is 1 m or less from it: move it first (hover over the button to see why).

The tools (they work on the selection and in its order):

- **“Live”**: shows the colour, the dimmer, the strobe and the gobo of each fixture, and every pixel of bars and matrices.
- **“Lasso”**: selects by drawing around the fixtures instead of with a box.
- **“Select”**: click or box to select.
- **“Move”**: drag the fixtures (the whole selection moves) or the view.
- **“Grid”**: “Free”, 10 cm, 25 cm, 50 cm or 1 m: where the fixtures land when you move them.
- **“Look”**: how the selected fixtures look: “Round”, “Square” or “Rectangle”, and their **“Icon”** (see [Icons, colours and legend](patch.md#icons-colours-and-legend)).
- **“Arrange”**: places them in a “Line”, “Line, front to back”, “Circle”, “Grid, by rows”, “Grid, snake”, “Grid, by columns” or “Grid, snake by columns”, with the **Spacing (cm)** and the **Columns** next to it.
- **“Align”**: “Left”, “Centre”, “Right”, “Back”, “Middle”, “Front”, “Distribute across” and “Distribute front to back”.
- **15°** (right click: the other way) and **90°**: rotate the selection.
- **“Height and facing”**: where the selected moving heads hang, for [Follow](patch.md#height-and-facing-follow).

Moving heads have a small arrow on their edge (a notch, when the plan shows them small): the way they face. The names of fixtures that are very close together are hidden so they do not overlap: zoom in to read them. A **shield** at the top left marks a protected special effect (dim; red with the effects armed; with a pulsing ring while it fires) and a **padlock** at the top right, a parked fixture.

The **wheel** zooms, the **right button** or the middle one drag the view and a double click with them centres it. The buttons in the corner also zoom. A right click without moving the mouse opens the [fixture menu](patch.md#selecting).

> **Tip:** place the fixtures in the order you want the effects to run: select them from left to right and use “Arrange › Line”.

### Icons, colours and legend

Each fixture shows on the plan the icon of what it is: moving head, scanner, wash or PAR, LED bar, strobe, blinder, dimmer or conventional, laser, smoke, hazer, CO2 jet, flame, cold spark, confetti, bubbles, snow, fan, effect or other. Its fixture type says it, unless the name of its model says more: an “Effect” called “CO2 Jet” is a CO2 jet and a dimmer called “Blinder” is a blinder, but a moving head called “Beam” or “Wash” is still a moving head. A generic dimmer, which can stand in for any fixture, takes the icon of its own name (“CO2 1”, “FLAME 3”, “BLINDER1”). LED bars and matrices show their cells; the other fixtures with several cells (a blinder with several lamps, a strobe with several segments) have the icon beside their cells.

To choose it by hand, select the fixtures and use **“Look” › “Icon”**; **“Automatic”** goes back to its own. The icon chosen is saved with the show.

Each fixture type of the show also has its colour, on the icon and on the edge, the same every time you open the show: two types with the same icon are told apart by their colour. An icon chosen by hand on a fixture of the library doesn't change its colour. Live, the fixtures that are on show their light and the ones that are off, their icon.

The **legend**, at the top left of the plan, lists the fixture types with their icon, their colour and how many there are (the generic dimmers, by what they stand in for). A click on a type selects all its fixtures, also on the plan of Program; **Ctrl+click** or **Shift+click** adds them to the selection. A click on its title folds or unfolds it, and that is remembered. The stage is placed beside or below the legend, so that it doesn't cover any fixture.

### Isolating a group on the plan

In the “2D plan” view, a click on a group of the “Fixture groups” panel **isolates** it: the other fixtures are locked and faded on the plan, and nothing selects or moves them: not a click, the box, the lasso, the legend nor “Move” (a click on one does nothing, and a box can start on top of it). That makes it easy to select part of a group on a crowded plan and make smaller groups with it. Of the selection, only the fixtures of the group that were already selected stay. The isolated group has a thick border and an eye, and beside the legend of the plan a label tells which group you see and how many fixtures it has.

- **“All”** selects all the fixtures of the group, as does **Ctrl+click** on the group. **Ctrl+click** on another group adds only those of its fixtures that are also in the isolated one.
- In the legend, a click on a type selects only its fixtures in the group; the types that have none are faded.
- A click on another group isolates that one instead; a selection pattern of its menu isolates that group first. A click on a selection always selects all its fixtures: if some are locked, it first isolates the selection's group or, if it holds fixtures of other groups, ends the isolation.
- After **“Create group”**, the group stays isolated: you can go on splitting it into more groups.
- To end it: another click on the group, the **×** of the label or **Esc**. It also ends when you switch to “List” or “Channels”, open another show or delete the group. It isn't saved with the show.

In the “List” and “Channels” views, a click on a group still selects its fixtures, as always.

### Height and facing (Follow)

For [Follow](programar.md#follow-aiming-at-a-point) to aim all the moving heads at a point correctly, each one needs to know where it is. Place the heads on the plan where they really are, select them and press **“Height and facing”**. What you change applies at once to all the selected heads and is saved with the show:

| Field | What it is |
|---|---|
| **Height** | How many metres above the stage floor the axis of the head is (if not set: 4 m hung, 0 m standing). |
| **Mounted** | **“Hung”** (from a bar or a truss, upside down) or **“Standing”** (on the floor). |
| **Facing** | Where the beam goes with the pan in the centre and the tilt leaning forward. 0° is towards the audience and it turns clockwise; it is the same rotation as the one of the 15° and 90° buttons, and the arrow on the plan shows it. |
| **Pan travel** / **Tilt travel** | The degrees each axis travels. “Its definition” uses the ones in the fixture definition; if it does not include them, 540° and 270° are used, and you are warned in orange. |
| **Pan the other way** / **Tilt the other way** | If, with Follow, a head pans or tilts the opposite way. It only affects Follow; the Patch inversions are in [Limits](patch.md#limits). |

“(they differ)” means that the selected heads do not have the same value; when you change it, all of them take the new one.

## Limits

Select fixtures and press **“Limits”**:

- **Dimmer**, **Pan** and **Tilt**: the allowed range, from 0 to 100 %. For example, a dimmer that never goes above 80 % or a tilt that never points at the audience.
- **“Invert pan”**, **“Invert tilt”** and **“Swap pan and tilt”** (for heads hung upside down or rotated).
- **“Reset”**: no limits and no inverted or swapped axes.

With a single fixture, the **Channels** list lets you decide for each channel:

- **Fades**: whether it follows the fade times. Off, it jumps straight to its value (useful for colour wheels and gobos).
- **Dimmer**: whether it is scaled by the grand master, the group masters and the scene dimmers. The grand master, besides, only scales the light (see [Masters](directo.md#masters)).

Limits act on the output: scenes, effects and live edits keep their values, but the output respects the limits. With several fixtures you see the values of the first one that has each control, and what you change is applied to all the ones that have it.

## Protected special effects

A **protected special effect** only fires with the effects armed, from scenes, cuelists and chasers, and each fire lasts at most its maximum time. How they are armed and what fires them is in [Live](directo.md#protected-special-effects).

**Which ones are.** **CO2**, **flame**, **cold spark** and **confetti** machines are protected by themselves; **bubble** and **snow** machines are not (protect them by hand if you want). The kind comes from the fixture type, the words of its definition, the name of a generic dimmer (“CO2 1”, “FLAME 3”, “FIRE 2”, “PYRO 1”) or the icon chosen on the plan. The automatic rules only ever protect, they never take the protection away: changing the name or the icon never unprotects a fixture; a fixture that becomes one of those kinds is protected, and a message says so (“CO2 4 is now a protected special effect”); a CO2 you unprotected by hand stays unprotected even if you call it “CO2 9”.

> **Limits:** “C02” written with a zero is not recognised, and “FOCO2” is not a CO2. A machine that isn't recognised isn't protected: check the shield in the list and protect it by hand.

**“Special effect”** (the flame button, next to “Limits”, or “Special effect…” in the fixture menu) opens its settings for the selection:

- **“Protected special effect”**: turning it off asks first (“Take the protection away?”). Without it, the fixture goes out as any other channel: without arming, without a maximum time, and also from the programmer, Identify, the mixer and the FX.
- **“Maximum fire time”**: from 0.1 to 30 s (by default: CO2 3 s, flames 2 s, sparks 5 s, confetti 2 s, the others 3 s).
- **“Minimum wait between fires”**: from 0 to 60 s (1 s by default), from the release.
- **“Safety channel”**: **“None”** or the channel of the fixture that the machine needs to fire, with its **“Values from / to”** (from 0 to 255; “from” can't be above “to”). That channel only goes out within those values while the effect is armed and firing (“from”, if the scene gives it no value); otherwise 0. It can only be chosen when the selected fixtures have the same channels, and a one-channel fixture can't have one (“… needs another channel to fire”). If the fixture changes mode or definition and another channel is left in its place, the safety channel is taken away, and a message says so (“The safety channel of … was taken away”).
- **“As its kind (…)”**: goes back to the protection and the times of its kind, without a safety channel.

**Which channels fire.** All of them but pan, tilt and the safety one, whatever their type (intensity, effect, height…): the maximum time applies to them. Pan and tilt only go out with the effects armed and from scenes, with no time limit, and at 0 when disarmed: a scene that only aims the machine doesn't fire it. A machine with nothing but pan and tilt fires on all its channels but the safety one. A fixture across two universes (from a QLC+ show) never fires: the window warns about it; move it into a single universe.

In the list, a **shield** after the name; on the plan, a shield at the top left: dim normally, red with the effects armed and with a pulsing ring while it fires. **Identify** never lights them. [DMX levels](salidas.md#dmx-levels) and the live plan show what really goes out: a disarmed CO2 shows 0 even while its scene plays.

The protection is saved with the show, in its DMX Total Control section, and [importing the patch of another show](patch.md#importing-from-another-show) brings it. When you open an older show or a QLC+ show, a message says once which fixtures are now protected effects by their kind (“2 fixtures of this show are protected special effects now: …”); also a smoke machine whose model says “CO2” or a generic dimmer called “Fuego…” or “Fire…”. To change it, use “Special effect”. QLC+ doesn't read this protection: opened in QLC+, the show fires these fixtures from anywhere, without arming and without a maximum time. Earlier versions of DMX Total Control don't read it either: they fire these fixtures without arming and, when they save the show, they drop the protection set by hand, the times and the safety channel. Use this version on every computer of the show.

## Parking channels

A **parked** channel always goes out at a fixed value, above the scenes, the cuelists, the FX, the programmer, the mixer (Full, Flash…), the grand master and Stop all: the hazer at 30 % all night, a broken head off and still, the house lights. Esc and Blackout never unpark it.

In the [fixture menu](patch.md#selecting) (right click in the list or on the plan):

- **“Park as it is now”**: all its channels at what goes out now, so the grand master, the mixer and the limits of that moment are baked in. Not possible during the show stop or a fade to black (their light would stay parked for good).
- **“Park off”**: the intensity channels (dimmers and colours) at 0 and the others as they are now; a fixture without intensity channels (a smoke machine or a hazer from the library), all at 0. During the show stop or a fade to black, the other channels are parked at what the show sends underneath.
- **“Also in blackout”**: with Blackout, its parked intensities keep their value. Without it, they go dark with Blackout; pan, tilt, gobo and the other channels that aren't light always stay parked.
- **“Unpark”** and **“Unpark all”**.

In [Program](programar.md#parking), **“Park”** parks the values the programmer holds on the selected fixtures.

With parked fixtures, the header of the Patch shows **“N parked · Unpark all”** (in a narrow window, the padlock, the number and the button with only its icon). In the list, a **padlock** after the name (“Parked: N channels with a fixed value at the output”); on the 2D plan, of the Patch and of Program, a padlock at the top right. In [DMX levels](salidas.md#dmx-levels), a padlock on each parked channel.

- A protected special effect can only be parked **off**: “Park as it is now” or “Park” in Program park it at 0 and say so (“A protected special effect can only be parked off”). Parked, it never fires, even armed; a fire cut by parking shows in the log as “parked”. A fixture parked lit that becomes protected is parked off; if it is unprotected later, it stays parked at 0 until you unpark it. Unparked, it only fires after its scene is released and pressed again.
- Parked channels follow the fixture when you re-address it. Changing the mode of a fixture removes its parked channels (“The parked channels of Spot 3 were removed: its mode changed”), except for a protected effect, which stays parked off. Deleting a fixture removes its own. A parked pan of a fixture with pan and tilt swapped goes out on the tilt channel, like the pan.
- The [show stop](directo.md#show-stop) wins over parked channels: a fixture parked off lights up too, and a parked hazer stays parked. A [fade to black](directo.md#fade-to-black) takes down the parked channels without “Also in blackout”.

Parked channels are saved with the show, in its DMX Total Control section (importing the patch of another show doesn't bring them). When a show with parked channels opens they apply at once, and a message says which (“2 parked fixtures: HAZER, Spot 3 (Patch)”). Opening another show or a new one removes them. When the program closes they are let go (the stage goes to rest), but they stay in the show. QLC+ doesn't park channels: there they play as their scenes say, and the copy for QLC+ warns about it (“2 parked fixtures: QLC+ doesn't park channels”). Earlier versions of DMX Total Control don't apply them and, when they save the show, they drop them, and the show stop settings too.

> **Not tested with real equipment:** parking has not been tested with real interfaces, nodes or hazers.

## LED matrices and strips

**“LED matrix”** patches a matrix or a pixel strip in one go:

1. **Name**, **Width (pixels)** and **Height (pixels)** (height 1 for a strip).
2. **Colours**: the order of each pixel (RGB, GRB, BGR, BRG, GBR, RBG or RGBW).
3. **Wiring**: “Rows”, “Rows, snake”, “Columns” or “Columns, snake”.
4. **Pixel 1**: the corner where it starts (“Top left”, etc.).
5. **Universe** and **Address**.
6. Check the summary and the preview of the **pixel order**, and press **“Patch”**.

One fixture is created for each line of pixels, plus a group for the colour effects and its place on the plan. If a line needs more than one universe, make it shorter or wire it the other way.

## Importing from another show

**“Import”** brings part of another show (from DMX Total Control or from QLC+) into this one:

- **“Patch of another show…”**: its fixtures. They go to the same addresses if those are free; if not, to the next ones. They bring their special-effect protection, their times, their safety channel and their plan icon (what the other show says never unprotects a CO2), but not their parked channels.
- **“Scenes of another show…”**: its scenes, assigned to the fixtures of this show that match (by address and model, by name or by model). They go into groups with the same name.
- **“Patch and scenes of another show…”**: both.
- **“Patch from a visualizer (CSV)…”**: the fixture table your visualizer exports; see [Importing a patch from a visualizer](patch.md#importing-a-patch-from-a-visualizer).

## Importing a patch from a visualizer

If you design the rig in a visualizer, export its fixture table (CSV) and import it: DMX Total Control patches the same fixtures, on the same universes and addresses, with their names, their numbers and their place on the plan.

1. Press **“Import” › “Patch from a visualizer (CSV)…”** and choose the file.
2. The **“Import a patch”** window says how many fixtures the table has, of how many types and in which universes. Nothing changes until you press **“Import”**.
3. Under **“Fixture types”**, each type of the table (fixture, mode and channels) says what it becomes and how it was found in the library: **“Exact”** (the same model and mode), **“By channel count”** (the mode with those channels), **“Similar name”**, **“Remembered”** or **“Chosen by hand”**. A **“Similar name”** shows in orange for you to check: it is a model with a name like it, not the same one. If it isn't in the library, it is patched as a **“Generic dimmer”** with its channels, in orange: everything stays at its addresses all the same. Each type also shows the icon its fixtures will have on the plan; one patched as a generic dimmer takes the one its name in the table says (a “CO2 Jet” comes as a CO2 jet; a “Flame”, as a flame).
4. **“Change”** chooses another fixture and mode (**“Choose from the library…”**), a generic dimmer or **“Don't import”**. What you choose by hand is remembered for the next time you import that type.
5. Under **“Options”**:
   - **“Place them on the stage plan”**, with the **View**: **“From above”** (the audience at the bottom of the plan) or **“From the front”** (as the audience sees the rig: the highest fixtures at the top). The front view is suggested when the rig hangs at different heights, and the last one you used is remembered. The plan grows to hold the rig (it never shrinks). Each import that places them centres the rig on the plan again, also after you change its sides with “−” and “+”.
   - **Groups**: one for each fixture type, one for each layer (when the table has more than one) and the ones of the “Groups” column of the table.
6. Below the types are the problems, before importing: addresses another fixture or another row already uses (those rows are not imported), rows without a universe or address, universes above 128, addresses past 512, a second DMX break (not imported), fixture numbers another fixture already has or that are too high (they get another one), an inverted pan or tilt that is neither yes nor no, and the universes added to the show.
7. Press **“Import N fixtures”**. A message says how many are new and how many were updated, and any problem left.

**From each row** come the fixture and its mode, the universe and address, the name (the “Unit” column) and the fixture number: the one of the “Channel” column, else the one of the other fixture number column, else an automatic one. The height of each moving head, whether it stands (turned over in the table) or hangs, and its inverted pan or tilt go to its [height and facing](patch.md#height-and-facing-follow), whether you place them on the plan or not.

**Importing the table again** (after changing the rig in the visualizer) updates the same fixtures: it knows them by the identifier of each row of the table, or by the same name and fixture, and changes their mode, address, name, number and place on the plan. The ones no longer in the table stay, unless you tick **“Remove the N fixtures imported before that are no longer in the table”** (their names are shown below it). An icon you choose by hand afterwards is kept, unless the fixture its type is patched as changes.

> **Limits:** only the first DMX break of each fixture is imported, and of the rotations only whether a head is turned over is used (the facing is not imported). Seen from above, fixtures hung one over another end up together on the plan: to see them apart, use the front view.

## Deleting fixtures

Select them and press **“Delete”**. It asks first: their values are removed from all the scenes.
