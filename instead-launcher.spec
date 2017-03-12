Summary:	Game update/launch program for INSTEAD
Name:		instead-launcher
Version:	0.7.0
Release:	1%{?dist}
License:	GPLv2
URL:		https://github.com/instead-hub/instead-launcher
Source0:    %{name}_%{version}.tar.gz
Group:		Amusements/Games
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n) 
BuildRequires: qt4-devel zlib-devel
Requires: qt4 zlib instead

%description 
Instead-launcher provides GUI for simple installing, updating and launching of INSTEAD games

%prep
%setup -q

%build
qmake-qt4 PREFIX=/usr
make

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/usr/bin/ $RPM_BUILD_ROOT/usr/share/applications/
install -m 755 instead-launcher $RPM_BUILD_ROOT/usr/bin/
install -m 644 -p instead-launcher.desktop $RPM_BUILD_ROOT/usr/share/applications/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_datadir}/applications/*

