[Setup]
AppName=instead-launcher-0.0.1
AppVerName=instead-launcher 0.0.1
DefaultDirName={pf}\Pinebrush games\instead-launcher-0.0.1
DefaultGroupName=Pinebrush games
UninstallDisplayIcon={app}\instead-launcher.exe
OutputDir=.

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl

[Files]
Source: "*.exe"; DestDir: "{app}"
Source: "*.qm"; DestDir: "{app}"

[Icons]
Name: "{group}\instead launcher"; Filename: "{app}\instead-launcher.exe"; WorkingDir: "{app}"
Name: "{group}\Uninstall instead launcher"; Filename: "{uninstallexe}"

[CustomMessages]
CreateDesktopIcon=Create a &desktop icon
LaunchProgram=Launch &program
UninstallMsg=Uninstall instead-launcher
ru.CreateDesktopIcon=Создать &ярлык на рабочем столе
ru.LaunchProgram=Запустить программу
ru.UninstallMsg=Удалить instead-launcher
es.CreateDesktopIcon=Crear la &etiqueta sobre el escritorio
es.LaunchProgram=Cumplir el programa
es.UninstallMsg=Quitar el instead-launcher

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}

[Run]
Filename: {app}\instead-launcher.exe; Description: {cm:LaunchProgram}; WorkingDir: {app}; Flags: postinstall

[Icons]
Name: {commondesktop}\INSTEAD Launcher; Filename: {app}\instead-launcher.exe; WorkingDir: {app}; Tasks: desktopicon
Name: {group}\INSTEAD Launcher; Filename: {app}\instead-launcher.exe; WorkingDir: {app}
Name: {group}\{cm:UninstallMsg}; Filename: {uninstallexe}

[UninstallDelete]
Name: {app}; Type: dirifempty
Name: {pf}\Pinebrush games; Type: dirifempty



