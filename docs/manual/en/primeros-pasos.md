# First steps

![The first steps guide on the Patch page](../img/primeros-pasos.png)

## The guide

The first time you open the program, the **first steps guide** appears: a card at the bottom right with five steps (your fixtures, a look, a scene, Live and the playbacks). **“Next”** and **“Back”** change the step and take you to the page of each one; you can use the pages while the card is open. You close it with the **✕** and open it again in Options › Help › **“Start the guide”**.

In the first step, **“Open the practice show”** opens the “Primeros pasos” example (if you have unsaved changes, it first asks whether to save them). It already has four RGBW LED PARs in the fixture group `PARs`, two moving heads in `Móviles` and the empty group “Mis escenas” in Live, ready for your scenes.

## The complete session

In this first session you will go all the way: connect the output, patch some fixtures, build a look in the programmer, record it as a scene in a group, play it in Live, record a cuelist on a playback and save the show. It will take you about twenty minutes.

We will use fixtures from the generic library: four RGBW LED PARs and two moving heads. If you have other fixtures, look for their manufacturer and model instead of the generic ones.

> **Tip:** you can follow this guide with no DMX interface connected. You can see the output in Outputs › “DMX levels” and on the 2D plan in Program.

> **Tip:** with the practice show open you already have the fixtures and the group: skip to step 4 and use the fixture group `Móviles` where this page says `Heads`, and the group “Mis escenas” where it says `My scenes`.

## 1. Connect the output

1. Connect your DMX USB interface to the computer (or make sure the computer is on the same network as your Art-Net or sACN node).
2. Open **Outputs** (F7).
3. In the row of universe 1, open the drop-down that says “Not connected” and choose your interface.
4. If it does not appear, press **“Rescan”**.

The connected universe is marked in green. You will find more details in [Outputs](salidas.md).

## 2. Patch some fixtures

1. Open **Patch** (F5) and press **“Add fixtures”**.
2. In **Manufacturer**, choose **Generic** (you can type in “Search manufacturer…”).
3. In **Model**, choose **LED PAR RGBW**.
4. In the **Patch** panel on the right:
   - **Mode**: “5ch DRGBW”.
   - **Quantity**: 4.
   - **Universe**: 1. The **Address** is set automatically to the first free one.
   - Leave **“Make a group with them”** ticked and type the name `PARs` in **Group**.
5. Press **“Patch 4 fixtures”**. Below, you will see which addresses they use.
6. Without closing the window, now choose **Moving Head Spot**, mode “11ch Standard”, quantity 2, group `Heads`, and press **“Patch 2 fixtures”**.
7. Press **“Close”**.

> **Tip:** set each fixture to the same DMX address that the Patch list shows. To find a fixture on stage, select it and press **“Identify”**: it blinks in white.

## 3. Create a group in Live

Scenes are played in Live, organised in groups. Create one before recording:

1. Open **Live** (F1) and press **“New group”**.
2. Click the name of the column (“Group 1”), delete it, type `My scenes` and press Enter.
3. In the crossfade time drop-down of the column (it says “By scene”), choose **2 s**.

The group is **“Exclusive”**: only one scene plays at a time, and when you press another one it crossfades.

## 4. Build a look in the programmer

1. Open **Program** (F3).
2. In the fixtures panel, click the label of the **PARs** group: all four are selected.
3. In the **Quick** panel, press **100%** next to the **Dimmer** fader.
4. Click the **red** colour swatch.

The PARs light up in red as soon as you touch it: what you set in the programmer goes out at once, over everything else.

> **Tip:** the same with the command line: type `GROUP 1 @ FULL` and press Enter. `GROUP 1` is the first fixture group. See [the command line](programar.md#command-line).

## 5. Record the scene in the group

1. Press **“Record…”** (top right).
2. In **Record into**, leave **“New scene”**.
3. In **Name**, type `Red`.
4. In **Add to group**, choose **My scenes**.
5. Press **“Record”**.

Record a second look: with the PARs selected and the dimmer at 100 %, click the **blue** swatch, press **“Record…”**, name `Blue`, group **My scenes** and **“Record”**.

Before going to Live, press **“Reset all”** to clear the programmer. While the programmer holds values, they override the scenes and you would not see what you play.

> **Tip:** in the record window, tick **“Clear the programmer after recording”** and the programmer will clear itself every time you record. The option is remembered.

## 6. Play the scenes in Live

1. Open **Live** (F1).
2. Press **Red**: the tile lights up with the colour of the group and the PARs go red.
3. Press **Blue**: the group crossfades from red to blue in 2 seconds.
4. Move the **Master** of the column to lower the intensity of the whole group.
5. To release it, press the square in the column header (“Release this group”), or switch to **Stop** in the top bar, press **Blue** and go back to **GO**.

More in [Live](directo.md).

## 7. Record a cuelist on a playback

Cuelists advance cue by cue with GO. The fastest way to create one is to record straight onto a playback.

1. In **Program**, click the **Heads** label.
2. Press **“Beam on”** (dimmer at full and shutter open) and **“Centre”** (pan and tilt to the centre).
3. Open **Playbacks** (F10) and press **“Record”**: the button blinks in red.
4. Click playback **1**, which is empty: a new cuelist is created with cue 1.
5. Go back to **Program**, open **Features**, the **Position** tab, and drag the cross of the XY grid somewhere else.
6. In **Playbacks**, press **“Record”** and click playback 1 again: cue 2 is recorded, with only what has changed (the position).
7. Go back to **Program** and press **“Reset all”**.
8. In **Playbacks**, click the name of playback 1 to select it and press **Space** (or the big **GO** button): cue 1 plays. Press again: it goes to cue 2.

The playback fader is its intensity: if you raise it from zero, the cuelist starts. More in [Playbacks](playbacks.md).

## 8. Save the show

1. Press **Ctrl+S** (or the floppy disk in the top bar).
2. As it is the first time, choose a folder and a name: it is saved as a `.qxw` file.

From then on, Ctrl+S saves to the same file and keeps the version there was before the first save of each session as `.bak`. With autosave on (see [Options](opciones.md#autosave)), the program saves a recovery copy apart every few minutes, without touching your file: if it closes unexpectedly, it offers to recover the changes.

## What next…

- Build your operating screen in the [Show](show.md).
- Learn to use [palettes](programar.md#palettes) and [FX](programar.md#fx) in the programmer.
- Edit the cues, their times and their tracking in [Scenes](escenas.md#cuelists).
- Connect a MIDI controller in [Mappings](mapeos.md).
