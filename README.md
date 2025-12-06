# rf-powermon
Raspberry Pi commandline software for cheap USB RF power meters. It is written primarily as RadioLib development tool, to be used on test setups and for hardware-in-the-loop testing.

## Dependencies

* cmake >= 3.18
* lgpio library (https://abyz.me.uk/lg/lgpio.html)

## Building

Simply call the `build.sh` script.

## Usage

Start the program by calling `./build/rf-powermon`. Check the helptext `./build/rf-powermon --help` for all options. When called without arguments, it will assume some defaults which may or may not be correct for your setup.

## Remote control

When running, the program can be controlled via socket connection - see the included `read_sock.sh` script for an example usage with netcat. Control uses SCPI commands, with the following ones being supported:

* `POWER:READ?` - returnes the average power level in dBm
* `SYS:EXIT` - shuts down the program (same as pressing Ctrl+C)
* `*RST` - resets statistics (min/max/avg)
* `*IDN?` - returns an identification string

There is also a simple C library in `lib/rf-powermon-client` that can be used in other C/C++ programs to connect to the running application.

## TODO list

In order of priorities:

* get rid of lgpio dependency
* implement sampling rate control and/or decimation
* export timeseries in some reusable format
* debugging
