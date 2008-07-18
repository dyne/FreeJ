Name:		freej
Version:	0.10_git
Release:	1%{?dist}
Summary:	Real time video editing and VJ software

Group:		Applications/Multimedia
License:	GPLv3+
URL:		http://freej.dyne.org/
Source0:	freej.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: SDL-devel, libpng-devel, freetype-devel, fontconfig-devel, libogg-devel
BuildRequires: libvorbis-devel, libjpeg-devel, slang-devel, libtheora-devel,
BuildRequires: ffmpeg-devel, ffmpeg-compat-devel, bluez-libs-devel, fftw-devel
BuildRequires: jack-audio-connection-kit-devel, alsa-lib-devel, perl-HTML-Template
#Requires:	

%description
FreeJ is a vision mixer: an instrument for realtime video manipulation used in the fields of dance teather, veejaying, medical visualisation and TV.

FreeJ lets you interact with multiple layers of video, filtered by effect chains and then mixed together. Controllers can be scripted for keyboard, midi and joysticks, to manipulate images, movies, live cameras, particle generators, text scrollers, flash animations and more.
All the resulting video mix can be shown on multiple and remote screens, encoded into a movie and streamed live to the internet.

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc



%changelog
