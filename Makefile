# Convenience wrapper. All tutorial binaries live under units/.
.PHONY: all clean help run-% unit-%

all:
	$(MAKE) -C units all

help:
	$(MAKE) -C units help

clean:
	$(MAKE) -C units clean

unit-%:
	$(MAKE) -C units unit-$*

run-%:
	$(MAKE) -C units run-$* OPT=$(OPT)
