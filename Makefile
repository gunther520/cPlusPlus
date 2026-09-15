# Convenience wrapper. Tutorial binaries live under units/; the assignment under capstone/.
.PHONY: all clean help run-% unit-% capstone capstone-%

all:
	$(MAKE) -C units all

help:
	$(MAKE) -C units help
	$(MAKE) -C capstone help

clean:
	$(MAKE) -C units clean
	$(MAKE) -C capstone clean

unit-%:
	$(MAKE) -C units unit-$*

run-%:
	$(MAKE) -C units run-$* OPT=$(OPT)

capstone:
	$(MAKE) -C capstone all

capstone-%:
	$(MAKE) -C capstone $*
