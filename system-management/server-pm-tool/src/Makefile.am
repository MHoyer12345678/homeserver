ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS=server-pm-tool

server_pm_tool_SOURCES =		\
	main.cpp					\
	ServerPMTool.cpp			\
	ServerPMTool.h				\
	GPIOController.cpp			\
	GPIOController.h


server_pm_tool_CPPFLAGS = \
		$(GPIOD_CFLAGS)


server_pm_tool_LDADD	  = \
		$(GPIOD_LIBS)
