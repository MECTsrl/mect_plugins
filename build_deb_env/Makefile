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
# - desktop target --> make desktop_target  (desktop_toolchain)
# - host    target --> make host_target     (host_toolchain)
# - i.MX28  target --> make imx28_target    (imx28_cross_toolchain)
# - Rpi4    target --> make rpi4_target     (rpi4_cross_toolchain)

# -----------------------------------------------------------------------------
include Makefile_common

# -----------------------------------------------------------------------------
.PHONY: all clean_all distclean_all

all: downloads desktop_target host_target imx28_target rpi4_target
	@echo "---------> done $@"

clean_all: clean_downloads clean_desktop_target clean_host_target clean_imx28_target clean_rpi4_target
	@echo "---------> done $@"

distclean_all: distclean_downloads distclean_desktop_target distclean_host_target distclean_imx28_target distclean_rpi4_target
	@echo "---------> done $@"

# -----------------------------------------------------------------------------
.PHONY: desktop_target clean_desktop_target distclean_desktop_target

desktop_target:
	make -f Makefile_desktop target

clean_desktop_target:
	make -f Makefile_desktop clean_target

distclean_desktop_target:
	make -f Makefile_desktop distclean_target

# -----------------------------------------------------------------------------
.PHONY: host_target clean_host_target distclean_host_target

host_target:
	make -f Makefile_host target

clean_host_target:
	make -f Makefile_host clean_target

distclean_host_target:
	make -f Makefile_host distclean_target

# -----------------------------------------------------------------------------
.PHONY: imx28_target clean_imx28_target distclean_imx28_target

imx28_target:
	make -f Makefile_imx28 target

clean_imx28_target:
	make -f Makefile_imx28 clean_target

distclean_imx28_target:
	make -f Makefile_imx28 distclean_target

# -----------------------------------------------------------------------------
.PHONY: rpi4_target clean_rpi4_target distclean_rpi4_target

rpi4_target:
	make -f Makefile_rpi4 target

clean_rpi4_target:
	make -f Makefile_rpi4 clean_target

distclean_rpi4_target:
	make -f Makefile_rpi4 distclean_target

# -----------------------------------------------------------------------------
