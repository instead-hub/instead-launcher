[Setup]
AppName=INSTEAD Launcher
AppVerName=INSTEAD Launcher 0.5
DefaultDirName={pf}\Pinebrush games\INSTEAD-LAUNCHER
DefaultGroupName=Pinebrush games
UninstallDisplayIcon={app}\instead-launcher.exe
OutputDir=.
OutputBaseFilename=instead-launcher-0.5
AllowNoIcons=true

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: ru; MessagesFile: compiler:Languages\Russian.isl
Name: es; MessagesFile: compiler:Languages\Spanish.isl

[Files]
Source: "*.exe"; DestDir: "{app}"
Source: "qt_ru.qm"; DestDir: "{app}"

[CustomMessages]
CreateDesktopIcon=Create a &desktop icon
LaunchProgram=Launch &program
UninstallMsg=Uninstall INSTEAD Launcher
ru.CreateDesktopIcon=Создать &ярлык на рабочем столе
ru.LaunchProgram=Запустить программу
ru.UninstallMsg=Удалить INSTEAD Launcher
es.CreateDesktopIcon=Crear la &etiqueta sobre el escritorio
es.LaunchProgram=Cumplir el programa
es.UninstallMsg=Quitar el INSTEAD Launcher

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
Name: {localappdata}\instead\launcher.ini; Type: files


