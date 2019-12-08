# `gate`

If you're familiar with my
[`fate`](https://github.com/AgentTroll/fate) project, then
you'll know that this basically extends on the idea of
helping you track non-natural satellites (e.g. the ISS).

This is essentially a more fully-featured astronomy tool
for someone like me, who is more interested in the software
than the actual astronomy (although the astronomy is pretty
cool as well). 

# Building

Requires `cmake` installed.

``` shell
git clone https://github.com/AgentTroll/gate.git
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

# Credits

Built with [CLion](https://www.jetbrains.com/clion/)

Uses [SPICE Toolkit](https://naif.jpl.nasa.gov/naif/toolkit.html)