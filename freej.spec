Name:		freej
Summary:	Linux video DJ and mixing
Version:	0.10
Release:	9.20080726%{?dist}
Source:		ftp://ftp.dyne.org/freej/snapshot/freej-20080726.tar.gz
URL:		http://freej.dyne.org
License:	GPLv3
Group:		Applications/Multimedia
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
Patch0:		freej-20080720-with_skeleton.patch
Patch1:		de-script.patch
Patch2:		stack-patch.patch
Requires:	SDL SDL_ttf freetype slang
Requires:	fftw jack-audio-connection-kit
Requires:	bluez-libs unicap ffmpeg-libs
BuildRequires:	libtheora-devel SDL-devel libpng-devel libjpeg-devel
BuildRequires:	SDL_ttf-devel freetype-devel  libvorbis-devel slang-devel
BuildRequires:	fftw-devel jack-audio-connection-kit-devel unicap-devel
BuildRequires:	bluez-libs-devel nasm gcc-c++ byacc flex bison
BuildRequires:	ffmpeg-devel python-devel swig

%description
FreeJ is a digital instrument for video livesets, featuring realtime
rendering of multilayered video and chained effect filtering directly
on the screen. It plays with video4linux devices, DIVX/AVI files, and
PNG images, letting you dynamically apply on each layer a chain of
effect plugins.

NOTE: This package is hard-coded to require an MMX capable processor.

%package devel
Summary: Headers for developing programs that will use freej
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
This package contains the headers that programmers will need to develop
applications which will use libraries from freej.

%prep
%setup -q -n freej-20080726
%patch0 -p1
%patch1 -p1
%patch2 -p1

%build
%configure --enable-v4l --enable-flash --without-goom
# freej does not handle multithreaded building
# it simply won't build at all
%{__make} -j1
										
%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
rm -fr $RPM_BUILD_ROOT/usr/doc

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README AUTHORS ChangeLog KNOWN-BUGS NEWS TODO USAGE doc/*.txt doc/*.png doc/x11*
%{_bindir}/%name
%{_libdir}/%name
%{_datadir}/%{name}
%{_mandir}/man1/freej.1.gz

%files devel
%defattr(-,root,root)
%doc README
%{_includedir}/*.h
%{_datadir}/%{name}/*.glade

%changelog
* Tue Sep 23 2008 Yaakov M. Nemoy <loup@hexago.nl> - 0.10-9.20080726
- escaped macro in the changelog
- added patch to remove erros about nonexecutables with #!
- added (non-working) patch to remove warning about executable stack (more work needed upstream)

* Sun Sep 21 2008 Yaakov M. Nemoy <loup@hexago.nl> - 0.10-8.20080726
- removed explicit requires
- changed -devel's group to Dev/Libs
- Fixed the license to GPLv3 from GPL
- Removed a trailing . in the Summary.
- filled in a comment to explain the -j1 flag
- added docs to -devel package
- made BuildRoot compliant with Guidelines

* Sun Jul 26 2008 jeff <moe@blagblagblag.org> 0.10-7.20080726.blag.f9
- Requires: ffmpeg-libs
- BuildRequires: ffmpeg-devel

* Sun Jul 26 2008 jeff <moe@blagblagblag.org> 0.10-6.20080726.blag.f9
- Update to 20080726 which has ffmpeg fixes

* Sun Jul 20 2008 jeff <moe@blagblagblag.org> 0.10-5.20080720.blag.f9
- freej-20080720-with_skeleton.patch disables skeleton

* Sun Jul 20 2008 jeff <moe@blagblagblag.org> 0.10-4.20080720.blag.f9
- Update to 20080720 snapshot
- Requires: unicap
- BuildRequires: add gcc-c++ byacc flex bison unicap
- BuildRequires: remove libgoom2-devel
- Requires: remove SDL_goom libgoom2
- configure --without-goom
- %%{__make} -j1

* Sun Jun  1 2008 jeff <moe@blagblagblag.org> 0.10-0blag.f9
- Update to 0.10
- Add Requires: and BuildRequires:

* Mon Jan  1 2006 jeff <moe@blagblagblag.org> 0.8-0blag.fc6
- Update to 0.8
- Clean up spec

* Fri Mar 05 2004 jeff <moe@blagblagblag.org> 0.6-1blag.rh9
- Rebuild

* Wed Feb 04 2004 moe <moe@blagblagblag.org> 0.6-1blag.fc1
- Update to 0.6

* Fri Dec 19 2003 moe <moe@blagblagblag.org> 0.5.1-2blag.fc1
- Rebuild for fc1

* Tue Dec 02 2003 moe <moe@blagblagblag.org> 0.5.1-1blag
- Update to 0.5.1
- Remove pluggerpath patch

* Fri Oct 11 2003 moe <moe@blagblagblag.org> 0.5-3blag
- Rebuild
- Added pluggerpath patch so plugins can be in /usr/lib/freej instead
  of /usr/local

* Thu Feb 27 2003 Austin Acton <aacton@yorku.ca> 0.4.1-1plf
- 0.4.1
- buildrequires libnas

* Sat Jan 18 2003 Götz Waschk <waschk@linux-mandrake.com> 0.4-2plf
- fix installation
- drop lib* packages
- move plugins to the right dir
- autoconf 2.5 macro
- force regeneration of Makefiles with automake

* Fri Jan 17 2003 Austin Acton <aacton@yorku.ca> 0.4-1plf
- initial package (depends on divx4linux)
