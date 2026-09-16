# Convenience wrapper. Tutorial binaries live under units/; the assignment under capstone/.
.PHONY: all clean help check tsan run-% run-assign-% unit-% capstone capstone-%

all:
	$(MAKE) -C units all

help:
	$(MAKE) -C units help
	$(MAKE) -C capstone help

clean:
	$(MAKE) -C units clean
	$(MAKE) -C capstone clean

check:
	$(MAKE) -C units check
	$(MAKE) -C capstone check

tsan:
	$(MAKE) -C units tsan

unit-%:
	$(MAKE) -C units unit-$*

run-%:
	$(MAKE) -C units run-$* OPT=$(OPT) PIN=$(PIN)

run-assign-%:
	$(MAKE) -C units run-assign-$* OPT=$(OPT) PIN=$(PIN)

capstone:
	$(MAKE) -C capstone all

capstone-%:
	$(MAKE) -C capstone $*
