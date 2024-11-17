ACLOCAL_AMFLAGS=-I m4
SUBDIRS=src

minicomconfdir=$(sysconfdir)/minicom
minicomconf_DATA = conf/minirc.dfl

install-exec-hook:
	ln -s $(bindir)/minicom $(DESTDIR)$(bindir)/server-console
