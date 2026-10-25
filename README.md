# Zeal Boing Ball Demo

An Amiga Boing Ball inspired demo for Zeal 8-bit Computer

## Controls

<kbd>Enter</kbd> or <kbd>Start</kbd> - Exit Demo

## Compile

With [ZDE](https://github.com/zoul0813/zeal-dev-environment), install ZGDK
and build with the current Zeal OS CMake toolchain:

```
zde deps install zgdk
zde cmake
```

The build generates assets from the checked-in GIFs, preserving the conversion
flags encoded in their filenames. Edit the Aseprite sources and export their GIFs
before rebuilding. Generated assets stay in `build/assets/`; the program is
`bin/boing.bin`.
