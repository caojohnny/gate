# `gate`

If you're familiar with my
[`fate`](https://github.com/caojohnny/fate) project, then
you'll know that this basically extends on the idea of
helping you track non-natural satellites (e.g. the ISS).

This is essentially a more fully-featured astronomy tool
for someone like me, who is more interested in the software
than the actual astronomy (although the astronomy is pretty
cool as well).

`gate` supports computing the location of stars, objects
cataloged by NAIF with a NAIF ID, satellites with TLE data
and even objects that can be added into the program's
database using RA/Dec.

# Building

Requires `cmake` installed.

``` shell
git clone https://github.com/caojohnny/gate.git
mkdir gate/build && cd gate/build
cmake ..
make install -j4
```

Building the docs requires the `doxygen` and `graphviz`
packages installed. Docs can be turned on by using the
`-DBUILD_DOC=ON` when doing `cmake ..`. They are output
to `build/doc`.

An installation directory can be specified by using
`-DCMAKE_BUILD_PREFIX=/path/to/dir` when doing `cmake ..`.

# Usage

`gate` is a library first and foremost. However, its
capabilities are demonstrated with the `gatecli`
executable. Run `gatecli` and type in `HELP` to print
detailed help messages.

# Demo

![STAR AZEL 70890 CONT NOW compared to Stellarium](https://i.imgur.com/DJmvC06.jpg)

```
Loading command file 'setup'...
OBSERVER_LATITUDE = 47.000000
OBSERVER_LONGITUDE = -122.000000
Loaded kernel for file './kernels/hipparcos.bin'
Loaded kernel for file './kernels/naif0011.tls'
Loaded kernel for file './kernels/pck00010.tpc'
Loaded kernel for file './kernels/earth_fixed.tf'
Loaded kernel for file './kernels/earth_000101_200314_191222.bpc'
Loaded kernel for file './kernels/de435.bsp'
STAR_TABLE = HIPPARCOS

gate> HELP
You can Ctrl+C any time to halt continuous output

--- HELP ---
EXIT - Quits the command line
HELP - prints this message
LOAD <CMD | KERNEL | CSN> <filename> - loads a set of commands or a kernel or CSN from file
SET <option> <value> - sets the value of a particular option
GET <option> - prints the value of a particular option
SHOW <TABLES | FRAMES | CSN | BODIES | CALC> - prints the available table, frame, named star, body, or custom calc object names
STAR INFO <catalog number> - prints information for a star with the given catalog number
STAR AZEL <catalog number> <CONT | count> <ISO time | NOW> - prints the observation position for the star with the given catalog number
BODY INFO <naif id> - prints information for a body with the given NAIF ID
BODY AZEL <naif id> <CONT | count> <ISO time | NOW> - prints the observation position for the satellite with the given NAIF ID
SAT ADD <id> - adds a satellite with the given ID to the internal database (non persistent)
SAT REM <id> - removes the satellite with the given ID from the internal database
SAT INFO <id> - prints information for a satellite added with the given ID
SAT AZEL <id> <CONT | count> <ISO time | NOW> - prints the observation position for the satellite added with the given ID
CALC ADD <id> <RANGE> <RA deg> <DEC deg> [<RA_PM deg/yr> <DEC_PM deg/yr>] - adds a body with the given ID to the internal database (non persistent)
CALC REM <id> - removes a body with the given ID from the internal database
CALC INFO <id> - prints information for a custom calculated body with the given ID
CALC AZEL <id> <CONT | count> <ISO time | NOW> - prints the observation for position the calculated body added with the given ID

--- OPTIONS ---
OBSERVER_BODY
OBSERVER_LATITUDE
OBSERVER_LONGITUDE
STAR_TABLE
```

# Credits

Built with [CLion](https://www.jetbrains.com/clion/)

Uses [SPICE Toolkit](https://naif.jpl.nasa.gov/naif/toolkit.html)
