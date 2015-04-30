libdirectio
===========

Disable the filesystem buffer cache by preloading this library.[1](http://arighi.blogspot.co.uk/2007/04/how-to-bypass-buffer-cache-in-linux.html)

To compile the library simply run `make`. You can pre-load it using the `LD_PRELOAD` environment variable in this way:

```bash
# export LD_PRELOAD=$FULL_PATH_OF_YOUR_LIBRARY/libdirectio.so.0.1
```

Then you can run your brand-new direct I/O benchmark (typically `dd`) for block devices.

To unload the library and restore the standard access simply run:
```bash
# unset LD_PRELOAD
```

You can also use it only for one single command:
```bash
# LD_PRELOAD=/path_to/libdirectio.so.0.1 dd if=/dev/zero of=test bs=512 count=1000
```
