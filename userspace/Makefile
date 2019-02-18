.PHONY: all clean

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

vpath %.c   ../target/:./
vpath %.h   ../target/:./
vpath %.o   ./

CFLAGS = -Wall -std=gnu99 -fPIC
#CFLAGS += -I ../Src

all: libmpik.so

%.d: %.c
	@set -e; $(CC) -MM $(CFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@

%.o: %.c
	@$(RM) $@
	$(CC) $(CFLAGS) -o $@ -c $< 

libmpik.so:	libmpik.o
	$(LD) -shared -soname libmpik.so -o ../target/libmpik.so libmpik.o $(LDFLAGS)

#-include $(OBJS:.o=.d)

clean:
	@rm -rf *.o *.d *.err libmpik.so
