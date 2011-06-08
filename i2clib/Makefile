prefix=i586-mingw32msvc-
CC=$(prefix)cc
STRIP=$(prefix)strip

OUT=../out/

LIBNAME=i2clib
SUFFIX=.dll

LIB=$(OUT)$(LIBNAME)$(SUFFIX)

SRC = \
	src/dllmain.cpp \
	src/i2clib.cpp

%.o: %.cpp
	$(CC) -c $< -o $@

$(LIB): $(SRC:.cpp=.o)
	$(CC) $< -shared external/pmdll.a -o $@
	$(STRIP) $@
