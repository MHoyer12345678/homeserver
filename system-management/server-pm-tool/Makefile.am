ACLOCAL_AMFLAGS=-I m4
SUBDIRS=src

DISTCHECK_CONFIGURE_FLAGS = \
  --with-systemdsystemunitdir=$$dc_install_base/$(systemdsystemunitdir)

if HAVE_SYSTEMD
systemdsystemunit_DATA = \
		conf/setup-server-access.service
endif

minicomconfdir=$(sysconfdir)/minicom
minicomconf_DATA = conf/minirc.dfl

install-exec-hook:
	ln -s $(bindir)/minicom $(DESTDIR)$(bindir)/server-console
