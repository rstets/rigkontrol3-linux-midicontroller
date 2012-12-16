SRC=rigkontrol3d.c
EXE=rigkontrol3d
RULES_D=/lib/udev/rules.d
RULES=90-rigkontrol3.rules

all:
	gcc -o $(EXE) $(SRC) -lasound

install:
	install -g root -o root -m 644 ./$(RULES) $(RULES_D)/$(RULES)
	install -g root -o root -m 755 ./$(EXE) /lib/udev/$(EXE)

uninstall:
	rm -rf /lib/udev/$(EXE)
	rm -rf $(RULES_D)/$(RULES)

clean:
	rm -rf $(EXE)
