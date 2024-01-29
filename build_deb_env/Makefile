# -----------------------------------------------------------------------------
# MectSuite Qt Embedded SDK builder
#
# - Windows host + i.MX28 target --> https://github.com/MECTsrl/mect_plugins/tree/mect_suite_3.5/build_win_env
#
# Prerequisites:
# - Debian/GNU Linux 10.0/Buster amd64 (schroot, cdeboostrap)
# - apt-get install make wget sudo aptitude
#
# Make targets for GNU/Linux Debian 10/amd64
# - desktop target --> make desktop_all
# - host    target --> make host_all
# - i.MX28  target --> make imx28_all
# - Rpi4    target --> make rpi4_all
#
# For instance:
# $ time make desktop_all 2>&1 | tee make_desktop_all.log
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
