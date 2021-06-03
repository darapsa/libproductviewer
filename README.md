# Portable 3D viewer using OpenGL ES 2

# Building

## Getting and preparing for configuration

```sh
$ git clone git://darapsa.org/libproductviewer.git
$ cd libproductviewer
$ autoreconf --install
```

## Configuring for various target hosts, compiling, linking, and installing

```sh
$ ./configure (or use the platform specific wrappers, and adjust as necessary)
$ make # -jN (with N an integer number of parallel tasks you allow your computer to run for compiling this)
$ sudo make install
```
