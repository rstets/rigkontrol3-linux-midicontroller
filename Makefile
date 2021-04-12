SRC=rigkontrol3d.c
EXE=rigkontrol3d
RULES=90-rigkontrol3.rules
EXE_DIR=/usr/local/bin # NOTE: In case you ever need to change this one. Do not forget to update path to EXE in RULES file: RUN+="/usr/local/bin/rigkontrol3d
RULES_DIR=/etc/udev/rules.d

all:
	gcc -o $(EXE) $(SRC) -lasound

install:
	install -g root -o root -m 644 ./$(RULES) $(RULES_DIR)/$(RULES)
	install -g root -o root -m 755 ./$(EXE) $(EXE_DIR)/$(EXE)

uninstall:
	rm -rf $(EXE_DIR)/$(EXE)
	rm -rf $(RULES_DIR)/$(RULES)

clean:
	rm -rf $(EXE)
