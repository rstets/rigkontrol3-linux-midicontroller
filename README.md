# NI Rig Kontrol 3 MIDI Controller for Linux

Converts input events from RK3 footswitches to MIDI notes (and to MIDI CC for expression pedal).


- ORIGINAL AUTHOR: Xendarboh <xendarboh@gmail.com>
- ORIGINAL SOURCE: https://code.google.com/p/rigkontrol3-linux-midicontroller/

### INSTALLATION

```
sudo apt install libasound2-dev
make
sudo make install
```


### FILES

- 90-rigkontrol3.rules	udev rules
- rigkontrol3d.c		source for the rigkontrol3 daemon
- rigkontrol3d.sh		helper script that detaches rigkontrol3d from parent process so that udev does not kill it


#### Update

- Changed midi port from 1 to 16
- Updated udev rules to work on rpi os


/usr/local/bin/rigkontrol3d $env{DEVNAME}
