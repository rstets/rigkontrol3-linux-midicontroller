SRC=rigkontrol3d.c
EXE=rigkontrol3d
RULES=90-rigkontrol3.rules
EXE_DIR=/usr/local/bin
RULES_DIR=/etc/udev/rules.d

all:
	gcc -o $(EXE) $(SRC) -lasound

install:
	install -g root -o root -m 644 ./$(RULES) $(RULES_DIR)/$(RULES)
	install -g root -o root -m 755 ./$(EXE) $(EXE_DIR)/$(EXE)
	udevadm control --reload-rules

uninstall:
	rm -rf $(EXE_DIR)/$(EXE)
	rm -rf $(RULES_DIR)/$(RULES)
	udevadm control --reload-rules

clean:
	rm -rf $(EXE)
