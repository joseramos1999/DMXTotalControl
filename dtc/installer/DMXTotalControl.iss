; DMX Total Control - instalador para Windows (Inno Setup 6)
;
; No se compila a mano: lo lanza build-installer.sh (y la publicación en GitHub),
; que deja una instalación limpia en dist/ y pasa la versión y esa carpeta con
; /DAppVersion y /DSourceDir. El instalador queda en build/installer/.
;
; Licensed under the Apache License, Version 2.0

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\..\dist"
#endif

#define AppName "DMX Total Control"
#define AppExe  "dmxtotalcontrol.exe"
#define AppDir  "DMXTotalControl"
#define AppUrl  "https://github.com/joseramos1999/DMXTotalControl"

[Setup]
; El AppId identifica el programa entre versiones: no cambiarlo nunca, o las
; actualizaciones se instalarían al lado en vez de encima.
AppId={{187E321B-DC21-4607-94ED-DBEF900097DB}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppName}
AppPublisherURL={#AppUrl}
AppSupportURL={#AppUrl}/issues
AppUpdatesURL={#AppUrl}/releases
VersionInfoVersion={#AppVersion}
VersionInfoDescription=Instalador de {#AppName}
VersionInfoProductName={#AppName}
DefaultDirName={autopf}\{#AppDir}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
; Por defecto, solo para el usuario actual y sin pedir administrador (como
; instalaba compilar-e-instalar.bat, en %LOCALAPPDATA%\Programs): así las
; actualizaciones no piden permiso. El asistente deja elegir "para todos los usuarios".
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
OutputDir=..\..\build\installer
OutputBaseFilename=DMXTotalControl-Setup-{#AppVersion}
SetupIconFile=..\icons\dtc.ico
UninstallDisplayIcon={app}\{#AppExe}
UninstallDisplayName={#AppName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
; el idioma de Windows, sin preguntar si es uno de los del instalador
ShowLanguageDialog=auto
; Si el programa está abierto durante una actualización, se ofrece cerrarlo.
CloseApplications=yes
RestartApplications=yes
; Registra los shows .qxw (para el usuario o para todos, según la instalación).
ChangesAssociations=yes

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[CustomMessages]
spanish.ShowFile=Show de {#AppName}
english.ShowFile={#AppName} show
spanish.FtdiDrivers=Abrir la página de los drivers USB de FTDI (interfaces DMX USB como Enttec Open DMX o USB Pro)
english.FtdiDrivers=Open the page of the FTDI USB drivers (DMX USB interfaces such as Enttec Open DMX or USB Pro)
brazilianportuguese.ShowFile=Show do {#AppName}
brazilianportuguese.FtdiDrivers=Abrir a página dos drivers USB da FTDI (interfaces DMX USB como Enttec Open DMX ou USB Pro)
portuguese.ShowFile=Show do {#AppName}
portuguese.FtdiDrivers=Abrir a página dos controladores USB da FTDI (interfaces DMX USB como Enttec Open DMX ou USB Pro)
french.ShowFile=Show {#AppName}
french.FtdiDrivers=Ouvrir la page des pilotes USB FTDI (interfaces DMX USB comme Enttec Open DMX ou USB Pro)
italian.ShowFile=Show di {#AppName}
italian.FtdiDrivers=Apri la pagina dei driver USB FTDI (interfacce DMX USB come Enttec Open DMX o USB Pro)
german.ShowFile={#AppName}-Show
german.FtdiDrivers=Seite der FTDI-USB-Treiber öffnen (DMX-USB-Interfaces wie Enttec Open DMX oder USB Pro)

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "assocqxw"; Description: "{cm:AssocFileExtension,{#AppName},.qxw}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; (lo que queda de compilaciones anteriores de otros programas no se instala)
Source: "{#SourceDir}\*"; DestDir: "{app}"; Excludes: "Sample.qxw,*.nsi"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
; Los shows: en "Abrir con" siempre; como programa por defecto si se pide.
Root: HKA; Subkey: "Software\Classes\DMXTotalControl.Show"; ValueType: string; ValueName: ""; ValueData: "{cm:ShowFile}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\DMXTotalControl.Show\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#AppExe},0"
Root: HKA; Subkey: "Software\Classes\DMXTotalControl.Show\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#AppExe}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\.qxw\OpenWithProgids"; ValueType: string; ValueName: "DMXTotalControl.Show"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\.qxw"; ValueType: string; ValueName: ""; ValueData: "DMXTotalControl.Show"; Flags: uninsdeletevalue; Tasks: assocqxw

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
; Actualización desde el programa: lo lanza en silencio con /RELAUNCH=1 y se cierra, así que se vuelve a abrir.
; Como el usuario que lo usaba, no como administrador (una instalación para todos pide permisos).
Filename: "{app}\{#AppExe}"; Flags: nowait runasoriginaluser; Check: IsUpdateRelaunch
; Las interfaces DMX USB de FTDI necesitan su driver (Windows suele instalarlo solo al conectarlas).
Filename: "https://ftdichip.com/drivers/d2xx-drivers/"; Description: "{cm:FtdiDrivers}"; Flags: postinstall shellexec skipifsilent unchecked; Check: not FtdiDriverInstalled

[Code]
function FtdiDriverInstalled(): Boolean;
begin
  Result := RegKeyExists(HKLM, 'SYSTEM\CurrentControlSet\Services\FTDIBUS');
end;

function IsUpdateRelaunch(): Boolean;
begin
  Result := ExpandConstant('{param:RELAUNCH|0}') = '1';
end;
