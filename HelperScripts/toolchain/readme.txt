To build Keyhotee binary be backward compatible to old Linux systems we developed GCC toolchain
supporting older GLIBC (2.11), kernel 2.6.31 and having all needed packages preinstalled to its sysroot.
Application can be compatible to Debian Squeeze x64 (released in Feb 2011) or newer. Should work
also on other distros (not Debian specific).

The toolchain should be built using crosstool-ng tool and current directory contains .config file
specifying it. Then all Keyhotee dependencies (including ICU, QT, OpenSSL, boost) should be built
using this toolchain.
Most of preinstalled packages was needed by QT build.

Warning: After building OpenSSL as its prefix toolchain sysroot/usr must be chosen since there is no
other way to point different version of the library than the one installed in OS. 

Here is list of debian dependencies:
libbz2-1.0_1.0.5-6+squeeze1_amd64.deb
libbz2-dev_1.0.5-6+squeeze1_amd64.deb
libdrm2_2.4.21-1~squeeze3_amd64.deb
libdrm-dev_2.4.21-1~squeeze3_amd64.deb
libexpat1_2.0.1-7+squeeze1_amd64.deb
libexpat1-dev_2.0.1-7+squeeze1_amd64.deb
libfontconfig1_2.8.0-2.1_amd64.deb
libfontconfig1-dev_2.8.0-2.1_amd64.deb
libfreetype6_2.4.2-2.1+squeeze4_amd64.deb
libfreetype6-dev_2.4.2-2.1+squeeze4_amd64.deb
libgl1-mesa-dev_7.7.1-6_amd64.deb
libgl1-mesa-dri_7.7.1-6_amd64.deb
libgl1-mesa-glx_7.7.1-6_amd64.deb
libglib2.0-0_2.24.2-1_amd64.deb
libglib2.0-dev_2.24.2-1_amd64.deb
libncurses5_5.7+20100313-5_amd64.deb
libpcre3_8.02-1.1_amd64.deb
libpcre3-dev_8.02-1.1_amd64.deb
libphonon4_4.6.0really4.4.2-1_amd64.deb
libphonon-dev_4.6.0really4.4.2-1_amd64.deb
libreadline6_6.1-3_amd64.deb
libreadline6-dev_6.1-3_amd64.deb
libsqlite3-0_3.7.3-1_amd64.deb
libsqlite3-dev_3.7.3-1_amd64.deb
libuuid1_2.17.2-9_amd64.deb
libx11-6_1.3.3-4+squeeze1_amd64.deb
libx11-dev_1.3.3-4+squeeze1_amd64.deb
libx11-xcb1_1.3.3-4+squeeze1_amd64.deb
libx11-xcb-dev_1.3.3-4+squeeze1_amd64.deb
libxau6_1.0.6-1_amd64.deb
libxau-dev_1.0.6-1_amd64.deb
libxcb1_1.6-1+squeeze1_amd64.deb
libxcb1-dev_1.6-1+squeeze1_amd64.deb
libxcb-glx0_1.6-1+squeeze1_amd64.deb
libxcb-glx0-dev_1.6-1+squeeze1_amd64.deb
libxcb-icccm1_0.3.6-1_amd64.deb
libxcb-icccm1-dev_0.3.6-1_amd64.deb
libxcb-image0_0.3.6-1_amd64.deb
libxcb-image0-dev_0.3.6-1_amd64.deb
libxcb-keysyms1_0.3.6-1_amd64.deb
libxcb-keysyms1-dev_0.3.6-1_amd64.deb
libxcb-randr0_1.6-1+squeeze1_amd64.deb
libxcb-randr0-dev_1.6-1+squeeze1_amd64.deb
libxcb-render-util0_0.3.6-1_amd64.deb
libxcb-render-util0-dev_0.3.6-1_amd64.deb
libxcb-shape0-dev_1.6-1+squeeze1_amd64.deb
libxcb-shm0_1.6-1+squeeze1_amd64.deb
libxcb-shm0-dev_1.6-1+squeeze1_amd64.deb
libxcb-sync0_1.6-1+squeeze1_amd64.deb
libxcb-sync0-dev_1.6-1+squeeze1_amd64.deb
libxcb-xfixes0-dev_1.6-1+squeeze1_amd64.deb
libxcomposite1_0.4.2-1_amd64.deb
libxcomposite-dev_0.4.2-1_amd64.deb
libxdamage1_1.1.3-1_amd64.deb
libxdamage-dev_1.1.3-1_amd64.deb
libxdmcp6_1.0.3-2_amd64.deb
libxdmcp-dev_1.0.3-2_amd64.deb
libxext6_1.1.2-1+squeeze1_amd64.deb
libxext-dev_1.1.2-1+squeeze1_amd64.deb
libxfixes3_4.0.5-1+squeeze1_amd64.deb
libxfixes-dev_4.0.5-1+squeeze1_amd64.deb
libxi6_1.3-8_amd64.deb
libxi-dev_1.3-8_amd64.deb
libxml2_2.7.8.dfsg-2+squeeze8_amd64.deb
libxrender1_0.9.6-1+squeeze1_amd64.deb
libxrender-dev_0.9.6-1+squeeze1_amd64.deb
libxslt1.1_1.1.26-6+squeeze3_amd64.deb
libxslt1-dev_1.1.26-6+squeeze3_amd64.deb
libxxf86vm1_1.1.0-2+squeeze1_amd64.deb
libxxf86vm-dev_1.1.0-2+squeeze1_amd64.deb
mesa-common-dev_7.7.1-6_amd64.deb
uuid-dev_2.17.2-9_amd64.deb
x11proto-core-dev_7.0.16-1_all.deb
x11proto-damage-dev_1.2.0-1_all.deb
x11proto-input-dev_2.0-2_all.deb
x11proto-render-dev_0.11-1_all.deb
zlib1g_1.2.3.4.dfsg-3_amd64.deb
zlib1g-dev_1.2.3.4.dfsg-3_amd64.deb

