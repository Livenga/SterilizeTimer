XC     = xc8
CHIP   = 16f887
BINDIR = bin
OBJDIR = obj
PRJC   = SterilizeTimer

SRC     = $(shell find src -name \*.c)
OBJS    = $(addprefix $(OBJDIR)/, $(patsubst %.c,%.p1,$(SRC)))
OBJDIRS = $(dir $(OBJS))

default:
	[ -d $(BINDIR) ] || mkdir -v $(BINDIR)
	[ -d $(OBJDIR) ] || mkdir -v $(OBJDIR)
	[ -d "$(OBJDIRS)" ] || mkdir -pv $(OBJDIRS)
	make $(PRJC)

$(PRJC):$(OBJS)
	$(XC) -O$@ --outdir=$(BINDIR) --chip=$(CHIP) $^

$(OBJDIR)/%.p1:%.c
	$(XC) -O$@ --outdir=$(dir $@) --chip=$(CHIP) --pass1 $<

clean:
	rm -rv $(BINDIR)
	rm -rv $(OBJDIR)

all:
	make clean
	make
