# Duality

![ROM](https://github.com/xythobuz/Duality/actions/workflows/build.yml/badge.svg)
![Web](https://github.com/xythobuz/Duality/actions/workflows/docs.yml/badge.svg)

My first homebrew GameBoy game.
A port of the GTA San Andreas Arcade Game Duality.

[Try it out online!](https://xythobuz.github.io/Duality)

[![Cartridge](https://xythobuz.github.io/Duality/cartridge.png)](https://xythobuz.github.io/Duality)

## Getting Started

You need the [GBDK-2020](https://gbdk.org/docs/api/docs_getting_started.html) to build the ROM and [Gearboy](https://github.com/drhelius/Gearboy), [SameBoy](https://sameboy.github.io/), [Emulicious](https://emulicious.net/) or [BGB](https://bgb.bircd.org/) to emulate it comfortably.
Then just build a debug version and run it in the emulator, with debug symbols already loaded.

    make run
    make sgb_run
    make bgb_run

Use SameBoy to test out the SGB border feature.
Also see below for symbolic debugging with Emulicious.

For the release build, simply add `GBDK_RELEASE=1` to your make invocation after running `make clean`.

    make clean
    make GBDK_RELEASE=1 run

You can also directly write to a flashcart using `flashgbx` with `make flash`.

## IDE Integration

I'm using [https://kate-editor.org/] which supports VSCode-style LSP and debugging with integrated plugins.

To generate a `compile_commands.json` for LSPs using `bear` run:

    make compile_commands.json

You can also debug using [Emulicious](https://emulicious.net/).
Add something like the following to your Kate Debugger user configuration:

    {
        "dap": {
            "emulicious": {
                "url": "https://emulicious.net/",
                "run": {
                    "command": ["emulicious", "-remotedebug", "${#run.port}"],
                    "port": 0,
                    "redirectStderr": true,
                    "redirectStdout": true,
                    "supportsSourceRequest": true
                },
                "configurations": {
                    "launch (debug)": {
                        "request": {
                            "command": "launch",
                            "mode": "debug",
                            "program": "${file}",
                            "cwd": "${workdir}",
                            "stopOnEntry": true
                        }
                    }
                }
            }
        }
    }

Unfortunately you will have to edit the hard-coded paths in `.vscode/launch.json`, I haven't been able to use variables there for some reason.

## License

The source code of this Duality GameBoy clone is licensed as GPLv3.
A copy of the license can be found in `COPYING`.

It uses the [GBDK-2020](https://gbdk.org) libraries and is based on their example code.
The files `sgb_border.c` and `sgb_border.h` are copied directly from their `sgb_border` example.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <http://www.gnu.org/licenses/>.

The `util/cvtsample.py` script is based on a [GBDK example](https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/wav_sample/utils/cvtsample.py).

The included cartridge label graphic in `artwork/cart_label.xcf` is based on the ['Cartridge-Label-Templates' by Dinierto](https://github.com/Dinierto/Cartridge-Label-Templates) licensed as CC0.

The included cartridge graphic in `artwork/cartridge.xcf` is based on the ['Front-End-Assets' by Duimon](https://github.com/Duimon/Front-End-Assets).
