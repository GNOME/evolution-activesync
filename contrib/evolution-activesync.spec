Name:		evolution-activesync
Version:	0.92
Release:	1%{?dist}
Summary:	An Evolution plugin to access ActiveSync servers

Group:		Applications/Productivity
License:	LGPL2.1 and Apache
URL:		https://github.com/GNOME/evolution-activesync
Source0:	http://ftp.gnome.org/pub/GNOME/sources/%{name}/%{version}/%{name}-%{version}.tar.xz

BuildRequires:	evolution-devel libwbxml-devel check-devel gcc-c++
BuildRequires:  libgnome-keyring-devel dbus-glib-devel GConf2-devel
Requires:	evolution libwbxml

%description
An Evolution plugin to access ActiveSync servers


%package devel
Summary:        Development files for evolution-activesync
Group:          Development/Libraries
License:	LGPL2.1 and Apache

%description devel
Development libraries for Evolution ActiveSync


%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}/%{_libdir}/evolution-data-server/camel-providers/libcameleas.la
rm -f %{buildroot}/%{_libdir}/evolution-data-server/registry-modules/module-eas-backend.la
rm -f %{buildroot}/%{_libdir}/evolution-data-server/libevoeas.la
rm -f %{buildroot}/%{_libdir}/evolution/modules/module-eas-mail-config.la
rm -f %{buildroot}/%{_libdir}/libeas*.la


%files
%doc
%{_libdir}/evolution-data-server/camel-providers/libcameleas.so
%{_libdir}/evolution-data-server/camel-providers/libcameleas.urls
%{_libdir}/evolution-data-server/registry-modules/module-eas-backend.so
%{_libdir}/evolution-data-server/libevoeas.so.0
%{_libdir}/evolution-data-server/libevoeas.so.0.0.0
%{_libdir}/evolution/modules/module-eas-mail-config.so
%{_libdir}/libeas.so.0
%{_libdir}/libeas.so.0.0.0
%{_libdir}/libeasaccount.so.0
%{_libdir}/libeasaccount.so.0.0.0
%{_libdir}/libeasclient.so.0
%{_libdir}/libeasclient.so.0.0.0
%{_libdir}/libeastest.so.0
%{_libdir}/libeastest.so.0.0.0
%{_libexecdir}/activesyncd
%{_datadir}/dbus-1/services/org.meego.activesyncd.service
%{_datadir}/locale/*/LC_MESSAGES/activesyncd.mo
%{_datadir}/glib-2.0/schemas/org.meego.activesyncd.account.gschema.xml
%{_datadir}/glib-2.0/schemas/org.meego.activesyncd.gschema.xml


%files devel
%{_includedir}/eas-daemon/
%{_libdir}/evolution-data-server/libevoeas.so
%{_libdir}/libeas.so
%{_libdir}/libeasaccount.so
%{_libdir}/libeasclient.so
%{_libdir}/libeastest.so
%{_libdir}/pkgconfig/libeasaccount.pc
%{_libdir}/pkgconfig/libeasclient.pc
