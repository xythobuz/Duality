# Duality

![ROM](https://github.com/xythobuz/Duality/actions/workflows/build.yml/badge.svg)
![Web](https://github.com/xythobuz/Duality/actions/workflows/docs.yml/badge.svg)

My first homebrew GameBoy game.
A port of the GTA San Andreas Arcade Game Duality.

[Try it out online!](https://xythobuz.github.io/Duality)

## Getting Started

You need the [GBDK-2020](https://gbdk.org/docs/api/docs_getting_started.html) to build the ROM and [Gearboy](https://github.com/drhelius/Gearboy) to emulate it comfortably.
Then just build a debug version and run it in the emulator, with debug symbols already loaded.

    make run

For the release build, simply add `GBDK_RELEASE=1` to your make invocation after running `make clean`.

## License

This Duality GameBoy clone is licensed as GPLv3.
A copy of the license can be found in `COPYING`.

It uses the [GBDK-2020](https://gbdk.org) libraries and is based on their example code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <http://www.gnu.org/licenses/>.
