# -----------------------------------------------------------------------------
# MectSuite Qt Embedded SDK builder
#
# Make QtCreator/Qt targets for GNU/Linux 
#
# - desktop target --> make desktop_all
# - host    target --> make host_all
#
# - i.MX28  target --> make imx28_all
# - Rpi4    target --> make rpi4_all
#
# Prerequisites: Debian/GNU Linux 11.0/bullseye amd64 (schroot, cdeboostrap)
#
#   (1.1) create the bullseye/amd64 chroot from a recent Debian/amd64 or Devuan/amd64 or Ubuntu/amd64 
#       you@host:~$ sudo cdebootstrap -f minimal bullseye /srv/chroot/bullseye_amd64_MectSuite_3.5 http://deb.debian.org/debian/
#           NB: using "minimal" the chrooted/home is different from /home and that's ok
#       you@host:~$ sudo vi /etc/schroot/schroot.conf
#           [bullseye_amd64_MectSuite_3.5]
#           description=Debian 12.0 Bullseye for amd64 MectSuite 3.5
#           directory=/srv/chroot/bullseye_amd64_MectSuite_3.5
#           #personality=linux32
#           root-users=you
#           type=directory
#           users=you
#
#   (1.2) install basic packages and create /home/you
#       you@host:~$ schroot -c chroot:bullseye_amd64_MectSuite_3.5 -u root
#       (bullseye_amd64_MectSuite_3.5)root@host:~# apt-get update && apt-get install make sudo aptitude git
#       (bullseye_amd64_MectSuite_3.5)root@host:~# mkdir /home/you
#       (bullseye_amd64_MectSuite_3.5)root@host:~# chown you:you /home/you
#       (bullseye_amd64_MectSuite_3.5)root@host:~# exit
#
#   (1.3) get this code
#       you@host:~$ schroot -c chroot:bullseye_amd64_MectSuite_3.5
#       (bullseye_amd64_MectSuite_3.5)you@host:~$ git clone https://github.com/MECTsrl/mect_plugins.git -b mect_suite_3.5
#
#   (2.1) make QtCreator2.8.1/Qt4.8.7 "desktop" target for GNU/Linux Debian/amd64 Bullseye
#       you@host:~$ schroot -c chroot:bullseye_amd64_MectSuite_3.5
#       (bullseye_amd64_MectSuite_3.5)you@host:~$ cd mect_plugins/build_deb_env
#       (bullseye_amd64_MectSuite_3.5)you@host:~/mect_plugins/build_deb_env$ make desktop_all
#
#   (2.2) use QtCreator2.8.1/Qt4.8.7 "desktop" on GNU/Linux Debian/amd64 Bullseye
#       you@host:~$ xhost +local:
#       you@host:~$ schroot -c chroot:bullseye_amd64_MectSuite_3.5
#       (bullseye_amd64_MectSuite_3.5)you@host:~$ export DISPLAY=:0
#       (bullseye_amd64_MectSuite_3.5)you@host:~$ /opt/qt-desktop-4.8.7/bin/qtcreator
#
# For developers:
# $ meld Makefile_desktop Makefile_host
# $ meld Makefile_host    Makefile_imx28
# $ meld Makefile_host    Makefile_rpi4
#

# -----------------------------------------------------------------------------
include Makefile_common

# -----------------------------------------------------------------------------
.PHONY: all clean_all distclean_all

all: downloads desktop_all host_all imx28_all rpi4_all
	@echo "---------> done $@"

clean_all: clean_downloads clean_desktop_all clean_host_all clean_imx28_all clean_rpi4_all
	@echo "---------> done $@"

distclean_all: distclean_downloads distclean_desktop_all distclean_host_all distclean_imx28_all distclean_rpi4_all
	@echo "---------> done $@"

# -----------------------------------------------------------------------------
.PHONY: desktop_all clean_desktop_all distclean_desktop_all

desktop_all:
	make -f Makefile_desktop all

clean_desktop_all:
	make -f Makefile_desktop clean_all

distclean_desktop_all:
	make -f Makefile_desktop distclean_all

# -----------------------------------------------------------------------------
.PHONY: host_all clean_host_all distclean_host_all

host_all:
	make -f Makefile_host all

clean_host_all:
	make -f Makefile_host clean_all

distclean_host_all:
	make -f Makefile_host distclean_all

# -----------------------------------------------------------------------------
.PHONY: imx28_all clean_imx28_all distclean_imx28_all

imx28_all:
	make -f Makefile_imx28 all

clean_imx28_all:
	make -f Makefile_imx28 clean_all

distclean_imx28_all:
	make -f Makefile_imx28 distclean_all

# -----------------------------------------------------------------------------
.PHONY: rpi4_all clean_rpi4_all distclean_rpi4_all

rpi4_all:
	make -f Makefile_rpi4 all

clean_rpi4_all:
	make -f Makefile_rpi4 clean_all

distclean_rpi4_all:
	make -f Makefile_rpi4 distclean_all

# -----------------------------------------------------------------------------
