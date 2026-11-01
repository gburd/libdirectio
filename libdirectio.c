/*
 * libdirectio: LD_PRELOAD shim that sets O_DIRECT on every regular file and
 * block device a program opens, so its I/O bypasses the page cache.
 *
 * Based on Andrea Righi's example:
 * http://arighi.blogspot.com/2007/04/how-to-bypass-buffer-cache-in-linux.html
 */
#define _GNU_SOURCE
#undef _FILE_OFFSET_BITS	/* keep open and open64 distinct symbols */
#undef _FORTIFY_SOURCE		/* fortify inlines would clash with our definitions */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* glibc's _FORTIFY_SOURCE entry points; not declared without fortify. */
int __open_2(const char *, int);
int __open64_2(const char *, int);
int __openat_2(int, const char *, int);
int __openat64_2(int, const char *, int);

#define NEEDS_MODE(f)	(((f) & O_CREAT) || ((f) & O_TMPFILE) == O_TMPFILE)
#define GET_MODE(flags, mode) do { \
	if (NEEDS_MODE(flags)) { \
		va_list ap; \
		va_start(ap, flags); \
		mode = va_arg(ap, mode_t); \
		va_end(ap); \
	} \
} while (0)
/* Resolved lazily: other constructors may call open() before ours runs. */
#define REAL(fn)	static __typeof__(fn) *real; if (!real) real = next(#fn)

static int debug;

__attribute__((constructor)) static void init(void)
{
	debug = getenv("DIRECTIO_DEBUG") != NULL;
}

static void *next(const char *sym)
{
	void *p = dlsym(RTLD_NEXT, sym);

	if (!p) {
		fprintf(stderr, "libdirectio: missing symbol %s\n", sym);
		abort();
	}
	return p;
}

/*
 * Turn on O_DIRECT after the open succeeds rather than passing it to open():
 * a filesystem that refuses O_DIRECT then just keeps buffered I/O instead of
 * failing the open (or, with O_CREAT|O_EXCL, creating the file and failing).
 * Only regular files and block devices: O_DIRECT on a FIFO means packet mode.
 */
static int directio(int fd, const char *path)
{
	int saved = errno, fl;
	struct stat st;
	const char *what = "skipped";

	if (fd < 0)
		return fd;
	if (fstat(fd, &st) == 0 && (S_ISREG(st.st_mode) || S_ISBLK(st.st_mode)) &&
	    (fl = fcntl(fd, F_GETFL)) != -1)
		what = fcntl(fd, F_SETFL, fl | O_DIRECT) == 0 ? "O_DIRECT" : "unsupported";
	if (debug)
		fprintf(stderr, "libdirectio: %s: %s\n", path, what);
	errno = saved;
	return fd;
}

int open(const char *path, int flags, ...)
{
	mode_t mode = 0;
	REAL(open);
	GET_MODE(flags, mode);
	return directio(real(path, flags, mode), path);
}

int open64(const char *path, int flags, ...)
{
	mode_t mode = 0;
	REAL(open64);
	GET_MODE(flags, mode);
	return directio(real(path, flags, mode), path);
}

int openat(int dirfd, const char *path, int flags, ...)
{
	mode_t mode = 0;
	REAL(openat);
	GET_MODE(flags, mode);
	return directio(real(dirfd, path, flags, mode), path);
}

int openat64(int dirfd, const char *path, int flags, ...)
{
	mode_t mode = 0;
	REAL(openat64);
	GET_MODE(flags, mode);
	return directio(real(dirfd, path, flags, mode), path);
}

int creat(const char *path, mode_t mode)
{
	REAL(creat);
	return directio(real(path, mode), path);
}

int creat64(const char *path, mode_t mode)
{
	REAL(creat64);
	return directio(real(path, mode), path);
}

int __open_2(const char *path, int flags)
{
	REAL(__open_2);
	return directio(real(path, flags), path);
}

int __open64_2(const char *path, int flags)
{
	REAL(__open64_2);
	return directio(real(path, flags), path);
}

int __openat_2(int dirfd, const char *path, int flags)
{
	REAL(__openat_2);
	return directio(real(dirfd, path, flags), path);
}

int __openat64_2(int dirfd, const char *path, int flags)
{
	REAL(__openat64_2);
	return directio(real(dirfd, path, flags), path);
}
