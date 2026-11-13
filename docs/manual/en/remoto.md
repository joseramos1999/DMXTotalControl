# Remote control

The **remote control** shows the [SHOW](show.md) pages in the browser of a phone or a tablet connected to the same Wi-Fi network as the computer. There is nothing to install: it works both ways, so what you touch on the phone changes on the computer and what changes on the computer shows on the phone.

## Turning it on

1. On the computer, open **Options** (F9) › **Remote control**.
2. Turn on **“Remote control on”**.
3. If you like, type a **PIN**. Without a PIN, anyone on the network can use it.
4. Below, the **addresses** of the computer appear (for example `http://192.168.1.20:8800/`), one for each network it is connected to, with the name of the network in front. Wi-Fi comes first. The copy button puts the address on the clipboard. If the computer has a **name on the network**, the first address is the name, for example `http://dmxtotalcontrol.local:8800/` (below).

## Connecting the phone or tablet

1. Connect the phone to the **same Wi-Fi network** as the computer.
2. Open the browser and type one of the addresses, or scan the QR code (below).
3. If there is a PIN, type it and press **OK**. It is asked only once on each device.

In Options you see how many devices are connected.

> **Tip:** add the page to the home screen of the phone: it opens like an app, with the icon of the program.

> **Tip:** if the phone does not connect, check that both are on the same network and that the Windows firewall lets DMX Total Control use the network. If another program uses the port, change it in Options (the warning appears in red).

> **Tip:** the first time you turn the remote control on, Windows asks whether DMX Total Control may use the network. Allow it on **private** networks. If Windows has the Wi-Fi as a **public** network, the phone cannot reach the computer: change it to private in the network settings of Windows.

> **Tip:** if the venue Wi-Fi does not let devices see each other, turn on the Windows **mobile hotspot** on the computer and connect the phone to it. The remote is then always at `http://192.168.137.1:8800/`.

## Scanning the QR code

With the remote control on, a **QR code** appears under the addresses. With it, the remote opens without typing anything:

1. Connect the phone or tablet to the **same Wi-Fi network** as the computer.
2. Point its camera at the code.
3. Tap the link that appears.

- The code holds the address with the coloured border: at first, the first numeric address, which works on every phone. To use another one, click its row or its QR code button.
- If there is a PIN, the code holds it too: the phone gets in without asking for it. The PIN does not stay in the address bar of the browser.
- So, with a PIN, anyone who takes a photo of the code can get in.
- **“Bigger”** shows the code large, to scan it from a couple of metres away.
- In the **Night** theme the code is dimmer, so that it does not dazzle.

## Finding it by name

The router may give the computer another address from one day to the next. The name does not change: with the remote control on, the phone opens `http://dmxtotalcontrol.local:8800/` even when the address is a different one. So the icon on the home screen keeps working.

If you open the remote with the QR code or a numeric address, the phone checks in the background whether it can reach the computer by its name. If it can, it moves to `http://dmxtotalcontrol.local:8800/` by itself, on the same page and without asking for the PIN. Add the icon to the home screen after that change: the address bar then shows the name.

- In Options, the **“By name”** address comes first in the list.
- Change the name in **Name on the network**. It takes letters, numbers and hyphens; capitals become lower case and spaces become hyphens. If it is empty, there is no name.
- It works on iPhone and iPad, and on phones with Android 12 or later. On older ones, use a numeric address or the QR code.
- It does not work when the phone has a VPN on or uses mobile data.
- If the phone cannot reach it by name, it stays on the numeric address without any warning.
- If the name stops answering later, the phone goes back to the numeric address, as long as that address answers.
- If another computer on the network already uses the name (for example, another DMX Total Control), this one takes the next one, `dmxtotalcontrol-2`, and Options says so under the name.
- If the network port 5353 is not available, or other computers already use the name and `-2` to `-9`, a warning appears in red. The numeric addresses still work.

## What you see

At the top is the remote bar:

- The **show name** and the **tabs** of the SHOW pages. The phone remembers the last page you opened.
- A **dot** that shows whether there is a connection with the computer.
- **BO**: blackout.
- The **full screen** button.

Below it, the page with its sections, as on the computer:

- **Buttons**: you press them with your finger, flash buttons too (while your finger is on them). They light up when they are active.
- **Faders**: you drag them; the ones with a **Flash** button have it below.
- **Dials and wheels**: you move them by dragging your finger up and down. Faders and dials follow the same [range](show.md#faders) as on the computer and show the real value.
- **Groups**: their scenes as buttons and their master.
- **Playbacks**: the strips with their name, the cue that plays and the next one, **GO**, **Back**, their fader and **Flash**. An A/B fader shows “A/B” and how far its fade is, with a dashed border. The sections that follow the current page have ◀ ▶ to change it.
- **Colour wheel** and **XY grid**, with their **Release** and **Centre** buttons.
- **Clock**, **Rhythm** (**GO**, **Stop** and **BeatGO**; the buttons of scenes with steps, of cuelists or of FX at “BPM of the show” hold their rhythm bar), **notes**, **playing now** and **image**.
- **Timer**: the same one as on the computer, with **Start** / **Pause** and **Reset**. If you start it on the phone, it runs on the computer too. If you pause it on the computer, it stops on every phone. In countdown it turns orange in the last minute and red when it reaches zero.
- **DMX levels**: the 512 channels of the chosen universe, as on the computer: the higher the value, the brighter the cell. The phone gets them about 8 times a second, only while that page is on screen.
- **Live mixer**: the same strips as the [mixer](directo.md#mixer) of the computer, side by side: “All fixtures” and one for each fixture group. If they do not fit, swipe them sideways. Each strip has the dimmer, **Full**, **Blackout**, **Hue**, **Saturation**, **Reset colour**, **Strobe** with its speed, **Flash**, **Flash excl.** and **Full excl.**
  - **Flash** and **Flash excl.** work while your finger is on them. They let go if the phone loses the connection, locks or you switch to another app.
  - Letting go turns them off, even if they were latched on the computer (Ctrl+click).
  - The border of a strip turns orange with a flash or a full, and red with blackout.
  - What you change on the phone changes on the computer, and the other way round.

If any playback has something assigned, the remote adds a **“Playbacks”** page with the strips of the current playback page.

The remote uses the same [theme](opciones.md#general) as the program and comes up in the language of the phone if it is Spanish, Portuguese, French, Italian or German (otherwise, in English).

## If the connection drops

If the phone loses the connection, “No connection with DMX Total Control. Trying again…” appears and it reconnects by itself. Whatever your finger was holding down (a flash, a button) is released on the computer, so that nothing stays on.
