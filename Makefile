.PHONY: all build clean wipe

all: build emu

build:
	yes | pebble build

clean:
	rm -Rf bin
	yes | pebble clean

wipe:
	yes | pebble wipe

run:
	pebble logs --emulator diorite &

emu:
	pebble install --emulator diorite
