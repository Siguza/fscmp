# fscmp

CLI frontend for com.apple.decmpfs / AppleFSCompression.framework.

### About

HFS and APFS support "decmpfs" attributes, which allow the file contents to be compressed on disk but be decompressed transparently on the fly when the file is read. Writing to such files will cause them to be decompressed.

Apple implements code for dealing with this layer in their "AppleFSCompression" PrivateFramework. This repository contains a reverse engineered header as well as other files required to link against that framework.

All code in this repository should be considered experimental, and is used at your own risk. The interface to AppleFSCompression.framework is not stable and may be changed at any time.

### Building

    make

macOS-only for obvious reasons. Maybe @saagarjha can be motivated to write a manual implementation for other OSes.

### Usage

    Usage: fscmp action [...]

    Actions:
        c[ompress] [options] file [file...]
        i[nfo] [options] file [file...]

    Options for compress:
        --  Treat all following arguments as path literals
        -1  Compression level 1
        -9  Compression level 9 (default)
        -B  Compression type LZBitmap
        -F  Compression type LZFSE (default)
        -R  Compression type raw (uncompressed, just store)
        -V  Compression type LZVN
        -Z  Compression type ZLib
        -p  Show progress (between files only)

    Options for info:
        --  Treat all following arguments as path literals
        -q  Quiet, just exit with 0 if all files are compressed, non-zero otherwise
        -t  Print only the compression type ID, one line per file

If you want to interface with AppleFSCompression.framework yourself, see [`main.c`](https://github.com/Siguza/fscmp/blob/master/src/main.c) for examples. That's as much as I know.

### License

[MIT](https://github.com/Siguza/fscmp/blob/master/LICENSE).

The heavy lifting is done by Apple code anyway.
