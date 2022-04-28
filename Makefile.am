ACLOCAL_AMFLAGS = -I m4

CXXFLAGS = -Wno-error -Wno-c++11-extensions -std=c++11
CFLAGS = -Wno-error
AUTOMAKE_OPTIONS = foreign dist-bzip2 no-dist-gzip 1.10

SUBDIRS = include src utilities samples

EXTRA_DIST = README.ja Doxyfile

# do "ldconfig" after "make install"
install-exec-hook : 
	@LDCONFIG@ $(libdir)

