## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

pmdn_ddsdocdir = ${prefix}/doc/pmdn_dds
pmdn_ddsdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS


EXTRA_DIST = $(pmdn_ddsdoc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(pmdn_ddsdocdir)
