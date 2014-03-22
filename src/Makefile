CC = wcc386
LD = wlink

RM = rm -f
CP = cp -u

CFLAGS = -bt=dos -za99 -6r -mf -oahkl+tx -fo=$^*.obj
LFLAGS = system dos32x option map

OBJS = &
    alloc.obj &
    dpmi.obj &
    error.obj &
    gfx.obj &
    image.obj &
    keyb.obj &
    main.obj &
    palette.obj &
    pcx.obj &
    res.obj &
    scene.obj &
    sprite.obj &
    timer.obj &
    vga.obj &
    xmalloc.obj &
    xmemcpy.obj

.suffixes:
.suffixes: .exe .obj .c .h

.c.obj: .autodepend
	$(CC) $(CFLAGS) $<

astroids.exe: $(OBJS)
	$(LD) $(LFLAGS) @$^*
	$(CP) $^@ ..

all: astroids.exe .symbolic

clean: .symbolic
	$(RM) *.obj
	$(RM) *.map
	$(RM) astroids.exe
