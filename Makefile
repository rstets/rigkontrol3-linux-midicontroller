all:
	gcc -o rigkontrol3d rigkontrol3d.c -lasound

install:
	install -g root -o root -m 644 ./rigkontrol3@.service /lib/systemd/system/rigkontrol3@.service
	install -g root -o root -m 644 ./90-rigkontrol3.rules /lib/udev/rules.d/90-rigkontrol3.rules
	install -g root -o root -m 755 ./rigkontrol3d /usr/local/bin/rigkontrol3d
	udevadm control --reload-rules
	systemctl daemon-reload

uninstall:
	rm -rf /lib/systemd/system/rigkontrol3@.service
	rm -rf /usr/local/bin/rigkontrol3d
	rm -rf /lib/udev/rules.d/90-rigkontrol3.rules
	udevadm control --reload-rules
	systemctl daemon-reload

clean:
	rm -rf rigkontrol3d
