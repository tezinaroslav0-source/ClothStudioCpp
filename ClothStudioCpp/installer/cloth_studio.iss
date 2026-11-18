#define AppName "Cloth Studio"
#define AppVersion "0.1.0"
#define AppExe "ClothStudio.exe"
[Setup]
AppId={{8AFA21D5-9B47-4B88-A0BD-3D1E9C7A5F10}
AppName={#AppName}
AppVersion={#AppVersion}
DefaultDirName={autopf}\Cloth Studio
DefaultGroupName=Cloth Studio
OutputDir=.
OutputBaseFilename=ClothStudio-Setup
ArchitecturesInstallIn64BitMode=x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
LanguageDetectionMethod=uilanguage
[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
[Files]
Source: "..\build\Release\ClothStudio.exe"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\Cloth Studio"; Filename: "{app}\ClothStudio.exe"
Name: "{autodesktop}\Cloth Studio"; Filename: "{app}\ClothStudio.exe"
[Dirs]
Name: "{userappdata}\ClothStudio\keys"
[Run]
Filename: "{app}\ClothStudio.exe"; Description: "Запустить Cloth Studio"; Flags: postinstall nowait skipifsilent
