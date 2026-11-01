libdirectio
===========

Bypass the page cache for any program by preloading this library. It sets
`O_DIRECT` on every regular file and block device the program opens, so
`dd`, `cp`, `cat`, etc. do direct I/O without being patched. Based on
[Andrea Righi's example](http://arighi.blogspot.com/2007/04/how-to-bypass-buffer-cache-in-linux.html).

Build and test (Linux, glibc):

```bash
make
make check
```

Use it for one command:

```bash
LD_PRELOAD=$PWD/libdirectio.so.0.2 dd if=/dev/sdX of=/dev/null bs=1M count=1000
```

or for a whole shell session:

```bash
export LD_PRELOAD=$PWD/libdirectio.so.0.2
# ... run your benchmark ...
unset LD_PRELOAD
```

Set `DIRECTIO_DEBUG=1` to log each file and whether `O_DIRECT` was applied.
`fincore FILE` shows whether a file's pages ended up in the page cache.

How it behaves
--------------

* `open`, `open64`, `openat`, `openat64`, `creat`, `creat64` and their
  `_FORTIFY_SOURCE` variants are wrapped.
* `O_DIRECT` is turned on with `fcntl()` after a successful open, and only
  for regular files and block devices. Character devices (`/dev/null`,
  `/dev/zero`), `/proc`, pipes and sockets are left alone. On a filesystem
  without `O_DIRECT` support (e.g. tmpfs) the file stays buffered; the open
  does not fail.
* `O_DIRECT` requires aligned buffers, offsets and sizes (usually 512 bytes
  or 4 KiB). Programs that do unaligned I/O, such as `echo hi > file`, get
  `EINVAL` on read/write. Use block-sized I/O (`dd bs=4096`).
* glibc's `fopen()` and other stdio calls open files internally and are not
  affected. Statically linked programs and raw `syscall()` callers are not
  affected either.

See also: [pagecache-management](https://code.google.com/archive/p/pagecache-mangagement)
and [nocache](https://github.com/Feh/nocache), which limit page cache use
through `posix_fadvise()` instead of `O_DIRECT`.
