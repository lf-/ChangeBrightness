# ChangeBrightness
A simple tool to change screen brightness from the Windows command line. This works on desktop computers as it uses the DDC system that is available on most screens made in the current millenium.

## Usage
```
Usage:
  ChangeBrightness.exe [OPTION...] brightness

  -a, --absolute        Set brightness to a given value
      --brightness arg  Amount to set brightness
  -h, --help            Print help
```

## Why?
Nirsoft ControlMyMonitor is great and can do the same thing as ChangeBrightness, but it, for whatever reason, takes over a second to apply the change. This program does not.

The intended use of this program is, for example, as a back end for brightness shortcuts on desktop machines.

Why use this instead of f.lux? Whatever f.lux is doing reduces the dynamic range of the monitor by actually darkening all the colours that are going to it. This program simply changes the hardware brightness.

## License
MIT
