# Options

![The Options page](../img/opciones.png)

**Options** (F9) brings together the program settings. On the left are the general settings, startup, lock, safety during the show, autosave, playback and audio; on the right, the remote control, the fixture library, updates, help and “About”.

The settings on this page belong to the computer, not to the show: they apply to every show you open. Only the show stop (in [Safety during the show](opciones.md#show-stop)) is saved with the show.

## General

- **Language**: “System language”, Español, English, Português, Français, Italiano or Deutsch. It is applied the next time the program starts.
- **Theme**: **Dark**, **Light**, **High contrast** or **Night (dimmed)**. It changes at once. Light, for rooms with light; High contrast, for reading from a distance; Night, very dark and in red, for a dark booth without dazzling you. The [remote control](remoto.md) follows the same theme.
- **Size of the interface**: from 80 % to 150 %, smaller for small laptops and bigger for touch screens. It is applied the next time the program starts.
- **Open the last show at startup**.

> **Tip:** the Light theme works better for daytime rehearsals or outdoors; High contrast, when there is a lot of light around the screen.

## SHOW and lock

- **Open the SHOW full screen at startup**: for installations and kiosks. The program starts on the Show page in full screen.
- **…and locked**: it also starts locked: only the SHOW, with no navigation. To leave, hold down the padlock button (with the password, if there is one). See [Show](show.md#full-screen-and-lock).
- **Lock screens**: **“Lock…”** / **“Unlock…”**. It is the same as the padlock in the top bar.

### Locking screens

Locking stops anything from changing by mistake during the show. Live and the SHOW keep working, and the playbacks keep playing.

1. Press the **padlock** in the top bar (or “Lock…” here).
2. Choose what to lock:
   - **Setup**: Patch, Fixtures, Outputs, Mappings and Options. You cannot open or create another show either.
   - **Programming**: Program and Scenes. In Playbacks you cannot record, update or assign.
   - **SHOW editing**: the SHOW pages are used, not changed.
3. If you like, type a **password** (it is optional).
4. Press **“Lock”**.

Locked pages have a padlock in the sidebar. When you open one, it asks for the password (if there is one) and everything is unlocked. To unlock, press the padlock in the top bar. **“Remove the password”** is in the same window, when you lock.

## Safety during the show

What protects the show while it is live (see [Safety during the show](directo.md#safety-during-the-show)):

- **Keep the computer awake** (on by default): while the program is open, Windows doesn't put the computer to sleep nor turn the screen off. When you close the program, or turn this off, Windows does it again as always. Nothing in the Windows settings changes.
- **Show mode** (off by default): closing the program or opening another show asks you to hold a button. Locking the SHOW turns it on; unlocking doesn't turn it off. The shield in the top bar turns it on and off as well (see [Show mode](directo.md#show-mode)).
- **Stop all with a fade**: from 0 to 5 s (0 by default: at once, as always). The functions fade out over that time; Esc twice, at once. For 10 seconds, Ctrl+Z brings back what it stopped (see [Stop all with a fade](directo.md#stop-all-with-a-fade)).
- **Disarm the special effects unused after**: from 0 to 120 minutes without a fire (10 by default; 0, “Never”). Never while one fires (see [Protected special effects](directo.md#protected-special-effects)).
- **Running on battery** and **USB selective suspend is on** only show when it happens. The first says how much battery is left; the second has **“How to turn it off”**, which opens the section [USB selective suspend](opciones.md#usb-selective-suspend) below.

### Show stop

What the [show stop](directo.md#show-stop) lights, and at what level. This one is saved with the show, in its DMX Total Control section (QLC+ doesn't read it).

- **Fixtures**: **“Every light”** (the default) or **“These groups…”**, which opens the list of the show's fixture groups to tick them (“Groups the show stop lights”). With none ticked, every light. A deleted group is dropped by itself. Of the chosen groups only the lights light up: their smoke, hazers, strobes and lasers never.
- **Level**: from 0 to 100 % (100 by default).

Moving heads, scanners, PARs and washes, LED bars, blinders and dimmers light up. A generic dimmer (Generic › Generic Dimmer) only when its name says it is a light (PAR, FRESNEL, DIMMER, BLINDER, LIGHT, LAMP, SPOT, FRONT, PROFILE, PC, HOUSE…) or its icon on the plan is that of a light. Those named VIDEO, TRIGGER, RELAY, MOTOR, SCREEN, PROJECTOR, KABUKI, CURTAIN… stay as they are, also inside a chosen group; a PYRO… is a protected special effect and goes dark. To light one of them, give it a light icon in Patch › “Look” › “Icon”. The dimmers of the library light up as always.

<a id="usb-selective-suspend"></a>

### USB selective suspend

Windows may suspend a USB port that seems idle to save power, and the DMX interface with it: the output stops in the middle of the show. It is a setting of the Windows power plan. The program only reads it at startup (and says so with a blue bar and on this page): it never changes it. To turn it off:

1. Open **Control Panel** › **Hardware and Sound** › **Power Options**.
2. Next to the plan you use, click **“Change plan settings”** and then **“Change advanced power settings”**.
3. Open **“USB settings”** › **“USB selective suspend setting”** and choose **“Disabled”** for **“On battery”** and for **“Plugged in”** (without a battery there is only one value).
4. Click **“OK”**.

If the interface still stops, open **Device Manager** and, in the properties of each **“USB Root Hub”**, on the **“Power Management”** tab, untick **“Allow the computer to turn off this device to save power”**. The names may change a little with the version of Windows.

> **Not tested with real equipment:** battery and power changes, the battery percentage, the real effect of keeping the computer awake and the reading of USB selective suspend.

## Autosave

- **Save a recovery copy**, **Every (minutes)**: from 1 to 60.

The autosave doesn't touch the show file: every few minutes, if there are unsaved changes, it saves a recovery copy apart, and the dot **●** stays. Only **Save** writes the file; the version there was before the first save of each session is kept as `.bak`. The copy is deleted when you save, when you close (saving or not) and when you open another show. If the program closes unexpectedly, when you open it again it offers the copy (“Recover the unsaved show?”) with the name of the show and the time of the copy: recovering it opens that show with its changes, still unsaved; save it to keep them. Meanwhile the last show is already open as it was saved (with “Open the last show at startup”): “Discard” leaves it so. The top bar shows the time of the last copy (“recovery copy …”); if it fails (a full disk), a warning appears in red.

## Playback

- **Default priority of new scenes**: from 0 to 10. If two scenes control the same channel, the one with the higher priority wins (see [Priorities](directo.md#priorities)).
- **Priority of the live edits**: what you set in Program beats the scenes with this priority or lower. With 10, all the scenes.
- **When a scene starts, the live edits**:
  - **“Give the scene its channels back”**: the channels that the scene touches leave the programmer.
  - **“Always stay”**.
  - **“Are all reset”**: the programmer is cleared.
- **Grand master acts on**: **“Intensity channels”** (the light: the dimmers and the colours of fixtures without a dimmer) or **“All channels”**. It never lowers the fixtures [protected from the grand master](patch.md#limits).

## Audio

- **Audio input**: the microphone or line input that **Pulse** and the audio matrices listen to. **“System default”** uses the one set in the operating system. Pulse changes input at once; an audio matrix that is already playing, the next time it starts. A chosen input that is not connected now shows as “(not connected)”, and meanwhile the default one is used.

## Recent shows

The list of the last shows you opened (also in the **▾** arrow of the top bar). **“Clear list”** clears it.

## Remote control

To use the SHOW pages from a phone or a tablet on the same Wi-Fi network, without installing anything:

- **Remote control on**.
- **Port**: change it if another program uses it (8800 by default). If it is in use, you are warned in red.
- **PIN**: asked once on each device. If it is empty, anyone on the network can use it.
- **Name on the network**: the phone opens the remote with `http://dmxtotalcontrol.local:8800/`, even when the address of the computer changes (iPhone, iPad and recent Android phones). If another computer already uses the name, this one takes `dmxtotalcontrol-2`. If it is empty, there is no name. If the network port 5353 is not available, or other computers use the name and `-2` to `-9`, you are warned in red.
- The **addresses** to open in the phone's browser, with a button to copy them. Each one has the name of its network in front, as Windows calls it (“Wi-Fi”, “Ethernet 2”). The **“By name”** address comes first, then Wi-Fi, then cable (Ethernet), and virtual adapters and VPNs last. The list updates by itself a few seconds after a network connects or disconnects. If the computer is on no network, a warning appears instead.
- A **QR code** of the address with the border (at first, the first numeric one), to open it with the camera of the phone. Click another address to change it. If there is a PIN, the code holds it and the phone gets in without asking for it. **“Bigger”** shows it large.
- How many devices are connected.

See [Remote control](remoto.md).

## Fixture library

It tells you how many fixture definitions are available (the built-in library, the generic fixtures and your own) and lets you extend it:

- **Open Fixture Library**: **“Download”** (needs internet access). Once downloaded, you see how many fixtures it has and when it was downloaded, and the button becomes **“Update”**: it brings the new and improved fixtures, and keeps your fixtures and the ones the library no longer has. If there is something new, you are told and the button is highlighted (see [Fixtures](fixtures.md#open-fixture-library)).
- **Import fixture files**: **“Import…”** `.qxf` definitions (the QLC+ format), `.gdtf` files or a `.zip` with them.
- **GDTF fixtures**: **“GDTF-Share…”** searches, downloads and imports fixtures from GDTF-Share (see [Fixtures](fixtures.md#gdtf-share)); **“Import GDTF…”** imports files, folders or the `.zip` of its multiple download. It tells you how many GDTF fixtures are in the library.
- **Fixtures downloaded from GDTF-Share**: appears when there are some. **“Convert again”** converts them again with this version of the program, from the `.gdtf` kept (see [Fixtures](fixtures.md#convert-again)).
- **Newer revisions on GDTF-Share**: shown when GDTF fixtures of the library have a newer revision on GDTF-Share. **“Update”** shows them in GDTF-Share, where **“Update all”** downloads the newest one of each and converts it: the fixtures patched with them get it. It counts the ones you downloaded from GDTF-Share. A file imported by hand counts if the text of its last revision matches exactly one revision on GDTF-Share. It uses the last GDTF-Share list the program has (see [Fixtures](fixtures.md#newer-revisions)).
- **Import my QLC+ fixtures**: **“Import”** copies the fixtures you created in QLC+.
- **User fixtures folder**: **“Open folder”**.

All of this is explained in [Fixtures](fixtures.md).

## Updates

- **Check for updates at startup**: once a day, on GitHub: the program and, if you downloaded the Open Fixture Library, whether it has new fixtures (a notice tells you). If you saved your GDTF-Share account, it also signs in to GDTF-Share and tells you if the fixtures you downloaded from it have newer revisions (each revision is told once). Nothing is downloaded or installed without asking you. With no internet, it does not bother you.
- **Include test versions**: the new features earlier, less tested.

Below, you see the status (“You have the latest version”, “Version … is out.”) and the buttons:

- **“Check for updates”**: checks now (the Open Fixture Library too).
- **“What's new”**: what is new in that version.
- **“Download and install”**: downloads the installer and checks its SHA-256 digest before saving it. If a version does not publish its digest, it can only be downloaded from its page (“Open on GitHub”).
- **“Install now”**: the program closes (it first asks whether to save the show), the new version is installed and the program opens again.
- **“When closing”** / **“Not when closing”**: install it when you close the program.
- **“Open on GitHub”**: the page of the version.

When there is a new version, a notice also appears above the pages with the same buttons. **“Later”** hides the notice for that version; the version is still available here.

## Help

- **Manual**: **“Open the manual”** opens this manual at its contents. F12 or the **“?”** button in the top bar open it on the page you are on. It opens in the language of the program (Spanish, English, Portuguese, French, Italian or German); the screenshots are those of the Spanish version.
- **First steps guide**: **“Start the guide”** opens the guide again (see [First steps](primeros-pasos.md#the-guide)).
- **Example shows**: they are in the **▾** arrow next to “Open” in the top bar, in “Examples” (see [the contents page](index.md#the-guide-and-the-example-shows)).

## About

The version of the program, its licence (free software, Apache 2.0) and the credits.
