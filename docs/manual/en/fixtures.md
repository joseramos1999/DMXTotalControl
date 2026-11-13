# Fixtures

![The Fixtures page](../img/fixtures.png)

To patch a fixture, the program needs its **definition**: which channels it has, what each one does and which modes it works in. This page explains where definitions come from and how to create your own on the **Fixtures** page (F6).

## The fixture library

When you add fixtures in the [Patch](patch.md#adding-fixtures) you choose from all these definitions:

- **The built-in library**: thousands of fixtures from many manufacturers, in `.qxf` format.
- **The generic fixtures**: 16 basic fixtures under the manufacturer **Generic** (RGB, RGBA, RGBW, RGBWA, RGBWA+UV and white LED PARs, an 8-segment LED bar, spot, beam and wash heads, scanner, strobe, blinder, laser, smoke machine and hazer), plus the **Generic Dimmer**.
- **The Open Fixture Library**, if you download it.
- **The GDTF fixtures** you download from GDTF-Share or import.
- **Your fixtures**: the ones you create on this page or import.

In [Options › Fixture library](opciones.md#fixture-library) you see how many definitions are available and you find everything you need to extend the library.

> **Tip:** for Program, FX and palettes to recognise the function of each channel (dimmer, red, pan, gobo…), the definition has to include the group or the preset of the channel. The ones in the library include it; in yours, choose the preset of each channel carefully.

## Creating a fixture

The **Fixtures** page is the fixture creator. On the left, **“My fixtures”** lists the ones you have created; on the right, the editor.

1. Press **“New fixture”**, or **“Start from the library…”** to start from a copy of one that already exists (it gets the same name with “(copy)”).
2. In **Fixture**, type the **Manufacturer** (for example `Generic` or the brand), the **Model** (`PAR 18x10W RGBWA`) and choose the **Type**.
3. Add the **channels** (see below).
4. Set up the **modes**.
5. Press **“Save fixture”**.

The fixture is available at once in the Patch, under its manufacturer. At the top you see its state: “Unsaved changes”, “Saved in your fixtures” or “Not saved yet”. If you open another fixture with unsaved changes, it asks whether to discard them (“Keep editing” / “Discard changes”).

### Channels

In **Channels**, the **“Add”** button offers the most common channels (dimmer, red, green, blue, white, amber, UV, pan and pan fine, tilt and tilt fine, strobe, colour wheel, gobo wheel, pan/tilt speed) and **“Custom channel”**.

On each channel:

- Its **name** and its **preset** (the drop-down): the preset says what the channel does. Changing the preset also changes the name, unless you have given it a name of your own.
- The bin deletes it.

A **“Custom”** channel also has:

- **Group**: its general function (intensity, colour, gobo, beam…).
- **Colour**: on an intensity channel, which colour it is (red, green…) or “None” for a dimmer.
- **Fine**: it is the fine channel (16 bits) of another one.
- **“Ranges (N)”**: what each range of DMX values does, for example 0-7 Closed, 8-255 Strobe. Press **“Add range”** and type its **Description**. Ranges cannot overlap. They are the presets you will later see as buttons in Program.

New channels are added at the end of the mode you have open.

### Modes

In **Modes**, **“Add”** creates a mode with all the channels. Each tab is a mode:

- The **Name** of the mode and the bin to delete it (there is always one left).
- The list is the **DMX order** of the mode: tick the channels it uses and move them with **▲ ▼** to their position. The number on the left is its DMX channel within the mode.

### Deleting one of your fixtures

Open the fixture in “My fixtures” and press **“Delete”**. It disappears from the library the next time the program starts.

Your fixtures are saved as `.qxf` files in the user fixtures folder (in Options › “User fixtures folder” › “Open folder”).

## Open Fixture Library

The Open Fixture Library is an open library with thousands of additional fixtures (MIT licence).

1. Open **Options** (F9).
2. In **Fixture library › Open Fixture Library**, press **“Download”**. You need internet access.
3. Wait for the progress bar to finish (“Cancel” stops it).

When it finishes, it tells you how many fixtures were downloaded and how many are new. Afterwards, the same place shows how many it has and when it was downloaded.

To bring it up to date, press **“Update”** (the same button). If “Check for updates at startup” is on in Options, once a day the program checks on GitHub whether the library has new or improved fixtures since your download, tells you and highlights the button; “Check for updates” checks it right away. The update:

- adds the new fixtures (they are listed below);
- changes the ones that have improved, also the ones you have patched (if their new mode does not fit where they are, they keep the previous version until you patch them again);
- keeps the ones the library no longer has, in case a show uses them;
- does not touch your fixtures or those of the built-in library: if they have the same manufacturer and model, yours (or the built-in one) stays.

A download that cannot be read, or that is cut off, changes nothing.

## GDTF-Share

GDTF-Share is the website where manufacturers publish their fixtures in GDTF. The program uses it with your own free account.

Open it with **“GDTF-Share…”** in **Options › Fixture library › GDTF fixtures**, or with **“Search GDTF-Share…”** in **Patch › Add fixtures**. At the bottom, **“gdtf-share.com”** opens its website.

### The account

Press **“Sign in”** and type your gdtf-share.com user name and password. They are only sent to gdtf-share.com. If you have no account, **“Create a free account”** opens its website.

- Without **“Remember on this computer”**, the password stays in memory only, until you close the program: it is asked for once per session.
- With it, it is saved encrypted for your Windows user: anyone using that Windows account can sign in with it.
- The user name is always remembered.

Once signed in, the button shows your user name. Its menu has **“Sign out”** (the saved password stays for the next time) and, if you saved it, **“Forget the saved password”**, which deletes it from the computer. After you sign out with a saved password, the **“Sign in”** button opens that menu: **“Sign in”** and **“Forget the saved password”**.

### Searching

The GDTF-Share list is saved on the computer and works without internet. Its date is at the top (“List of…”). It is downloaded again when it is more than 20 hours old, or with **“Refresh”** (the arrows). If GDTF-Share sends an empty list, the previous one stays.

- Type part of the manufacturer or the name, in any order.
- The filters: **“All”**, **“From manufacturers”** (uploaded by the manufacturer itself), **“In my library”** and **“Updates”** (those of your library with a newer revision).
- On the left, the manufacturers of what you find, and how many fixtures each one has.
- Each fixture shows who uploaded it (**Manufacturer** or **User**), its rating, and whether it is **In library**, has a **Newer revision** or you imported it **From a file**.
- At most 400 are shown: type more to narrow the list.

### Downloading and importing

Choose a fixture: its details are on the right (who uploaded it, the revision, the GDTF version, the size and the channels of each mode).

1. To get an older revision, choose it in **Revisions**.
2. Press **“Download and import”**. It is downloaded and converted as with “Import GDTF”, with a progress bar. **“Cancel”** stops it. If you download several at once, the ones that already arrived are still imported. The original `.gdtf` is kept (see “Convert again”, below).
3. From Add fixtures, **“Patch it”** chooses the fixture to patch (also one that was already in your library).

If the fixture is already in your library, the button is **“Update to this revision”** or **“Download again”**. If you have another upload of the same fixture, you are warned: the new one replaces it.

### Newer revisions

A fixture of your library has a newer revision when GDTF-Share has a later revision of the same fixture. This counts the ones you downloaded from GDTF-Share, and those imported from a file whose last revision has the same text as exactly one revision on GDTF-Share.

- In GDTF-Share, **“Updates”** lists them and **“Update all”** downloads the newest revision of each one. The fixtures patched with them get it.
- In **Options › Fixture library**, the row **“Newer revisions on GDTF-Share”** tells you how many there are, and **“Update”** opens GDTF-Share on “Updates”.
- With the saved password and **“Check for updates at startup”** on ([Options](opciones.md#updates)), the program signs in by itself once a day at startup, downloads the list and tells you if the fixtures you downloaded from GDTF-Share have newer revisions. Each revision is told once. If it can't, it says nothing.

### Convert again

The program keeps the `.gdtf` of each fixture you download from GDTF-Share. **“Convert again”**, in **Options › Fixture library › Fixtures downloaded from GDTF-Share**, converts them again with the version of the program you have, without downloading them: this way they get the improvements of a new version. The fixtures patched with them get the change. The row appears when there are fixtures downloaded from GDTF-Share.

## Importing GDTF

GDTF is the open format that manufacturers publish on GDTF-Share (with a free account).

1. Download the `.gdtf` files of your fixtures from gdtf-share.com (or the `.zip` of its multiple download). To search and download them from the program, see GDTF-Share, above.
2. In **Options › Fixture library › GDTF fixtures**, press **“Import GDTF…”** and choose the files, folders or the `.zip`.
3. They are converted in the background, with a progress bar. **“Stop”** stops the import.

When it finishes, it tells you how many were converted and, if any could not be read, why. If the same fixture is already in another library, the GDTF one is added as “(GDTF)” and you can use both. Importing it again updates it.

A mode with several *DMX breaks* (two DMX inputs, or a part on another universe) becomes several modes. The first break keeps the name of the mode. Each of the others is another mode, for example “Standard (DMX break 2)”, with only its channels, from address 1. Patch it as another fixture, at its own address: **“Add fixtures”** reminds you when you choose the mode.

Virtual channels (without DMX) are left out. They are not needed: an RGB fixture without a dimmer lights up through its colours, and the grand master lowers them.

Some channels change their function with the value of another channel (for example, the prism rotation only turns with the prism on “rotation”). In them the functions that do nothing are left out and, where there are several, each range carries their names together (for example, “0 deg / Stop”).

To see these changes on a fixture imported before, import its `.gdtf` again. For the ones downloaded from GDTF-Share, press “Convert again”.

## Importing fixture files

**Options › Import fixture files › “Import…”** accepts `.qxf` definitions (the QLC+ format), `.gdtf` files or a `.zip` that contains them. They are added to the library and it tells you how many are new.

### Importing my QLC+ fixtures

If you created fixtures in QLC+ on this computer, **Options › “Import my QLC+ fixtures” › “Import”** copies them to your fixtures.

## Formats that cannot be imported

Libraries of other programs in closed or encrypted formats cannot be imported. Most fixtures are in the Open Fixture Library or on GDTF-Share, which you can search from the program; for the ones that are missing, create them on this page.
