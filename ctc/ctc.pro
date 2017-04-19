TEMPLATE = app
LANGUAGE = C
TARGET = ctc

DESTDIR = .

SOURCES += ctc.c

HEADERS += ctc.h


PROJECTNAME = Cross Table Compiler

# Install the target.
target.path = $$QTC_PREFIX/bin
