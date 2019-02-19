.PHONY: all clean

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

vpath %.c   ../target/:./
vpath %.h   ../target/:./
vpath %.o   ./

CFLAGS = -Wall -std=gnu99 -fPIC
CFLAGS += -I ../kernel/ -I ../userspace/
LDFLAGS = -L ../target -l mpik -l pthread -l rt

all: example_1

%.d: %.c
	@set -e; $(CC) -MM $(CFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@

%.o: %.c
	@$(RM) $@
	@$(CC) $(CFLAGS) -o $@ -c $< 

example_1:	example_1.o
	@$(CC) -o ../target/example_1 example_1.o $(LDFLAGS)

#-include $(OBJS:.o=.d)

clean:
	@rm -rf *.o *.d *.err example_1
