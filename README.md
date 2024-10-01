# Midas

An attempt at algorithmic day trading, with the intention of supplementing one's income by a small yet steady daily amount.

## Requirements

This application has some dependencies such as intel's decimal floating point library and the IBKR API. These dependencies are packaged here for convenience but they each have their own license and user agreements which are included with the source code.
Outside of these third party libraries the GNU AGPLv3 license applies to all other works.
Third party libraries are contained in the subfolder `third-party`


## Goal
To provide a simple yet powerful means of running trading algorithms locally.


## Build instructions

Make sure you have boost libraries installed.

Executing the following commands in the project's root directory. Will build the application and related libraries

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build
```

## Testing instructions

After building run 
```bash
ctest --test-dir build
```


## Generating Documentation
We attempt (and hope) to provide and maintain adequate documentation via in code comments and doxygen. 
To generate simply run `doxygen` in the root of the repository. This will generate documentation files in different formats under docs folder. This folder is created if it does not exist.


