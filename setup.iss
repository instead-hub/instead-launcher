[Setup]
AppName=instead launcher
AppVerName=instead launcher 0.0.1
DefaultDirName={pf}\Pinebrush games\instead launcher 0.0.1
DefaultGroupName=Pinebrush games
UninstallDisplayIcon={app}\instead-launcher.exe
OutputDir=.

[Files]
Source: "*"; DestDir: "{app}"

[Icons]
Name: "{group}\instead launcher"; Filename: "{app}\instead-launcher.exe"; WorkingDir: "{app}"
Name: "{group}\Uninstall instead launcher"; Filename: "{uninstallexe}"
