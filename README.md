# powermon
Raspberry Pi commandline software for cheap USB RF power meters. It is written primarily as RadioLib development tool, to be used on test setups and for hardware-in-the-loop testing.

## Dependencies

* cmake >= 3.18
* lgpio library (https://abyz.me.uk/lg/lgpio.html)

## Building

Simply call the `build.sh` script.

## Usage

Start the program by calling `./build/powermon`. Check the helptext `./build/powermon --help` for all options. When called without arguments, it will assume some defaults which may or may not be correct for your setup.

## TODO list

In order of priorities:

* implement sampling rate control and/or decimation
* gitrev in CMake
* export timeseries in some reusable format
* allow control and data readout via sockets
* debugging
* CI
