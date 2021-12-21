Name:       yottagram

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}
Summary:    Yottagram
Version:    0.2.7
Release:    1
Group:      Qt/Qt
License:    GPLv3
URL:        http://verdanditeam.com/
Source0:    %{name}-%{version}.tar.bz2
Patch0:     disable-x11-and-desktop-capture.patch
Patch1:     force-mono-mic.patch
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(connman-qt5)
BuildRequires:  pkgconfig(vorbisfile)
BuildRequires:  pkgconfig(nemonotifications-qt5)
BuildRequires:  pkgconfig(libavcodec)
BuildRequires:  pkgconfig(opus)
BuildRequires:  pkgconfig(libjpeg)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(protobuf)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(alsa)
BuildRequires:  desktop-file-utils
Recommends:     voicecall-yottagram-plugin

%description
Fastest telegram client for sailfishos on the earth!

%package voicecall-plugin
Summary:        Yottagram's Voicecall plugin
BuildRequires:  voicecall-qt5-devel
Requires:       voicecall-qt5
Requires:       %{name} >= %{version}

%description voicecall-plugin
Yottagram's Voicecall plugin

%package transferengine-plugin
Summary:        Yottagram's Transfer Engine plugin
BuildRequires:  libnemotransferengine-qt5-devel
Requires:       libnemotransferengine-qt5
Requires:       %{name} >= %{version}

%description transferengine-plugin
Yottagram's Transfer Engine plugin

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 

%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}
%{_datadir}/dbus-1/services/com.verdanditeam.yottagram.service
%{_datadir}/lipstick/notificationcategories/x-verdanditeam.yottagram.im.conf
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%files voicecall-plugin
%{_libdir}/voicecall/plugins/libyottagram-voicecall-plugin.so

%files transferengine-plugin
%{_libdir}/nemo-transferengine/plugins/libYottagramTransferEnginePlugin.so
%{_datadir}/nemo-transferengine/plugins/YottagramShare.qml
