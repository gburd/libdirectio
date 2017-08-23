#define _GNU_SOURCE
#define __USE_GNU

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define DEBUG

#ifdef DEBUG
#define DPRINTF(format, args...) fprintf(stderr, "debug: " format, ##args)
#else
#define DPRINTF(format, args...)
#endif

int open(const char *, int, ...) __attribute__ ((weak, alias("wrap_open")));
int __open(const char *, int, ...) __attribute__ ((weak, alias("wrap_open")));
int open64(const char *, int, ...) __attribute__ ((weak, alias("wrap_open64")));
int __open64(const char *, int, ...) __attribute__ ((weak, alias("wrap_open64")));

static int (*orig_open)(const char *, int, ...) = NULL;
static int (*orig_open64)(const char *, int, ...) = NULL;

static int __do_wrap_open(const char *name, int flags, mode_t mode,
int (*func_open)(const char *, int, ...))
{
    if (!strncmp("/dev/", name, sizeof("/dev/") - 1) || !strncmp("/proc/", name, sizeof("/proc/") - 1)) {
        //return fd;
        DPRINTF("skipping flag O_DIRECT on %s\n", name);
        return func_open(name, flags, mode);
    }
    if (strncmp("/dev/null", name, sizeof("/dev/null"))) {
        DPRINTF("setting flags O_DIRECT on %s\n", name);
        flags |= O_DIRECT;
    }
    DPRINTF(" not setting flags O_DIRECT on %s\n", name);
    return func_open(name, flags, mode);
}

int wrap_open(const char *name, int flags, ...)
{
    va_list args;
    mode_t mode;

    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);

    DPRINTF("calling libc open(%s, 0x%x, 0x%x)\n", name, flags, mode);

    return __do_wrap_open(name, flags, mode, orig_open);
}

int wrap_open64(const char *name, int flags, ...)
{
    va_list args;
    mode_t mode;

    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);

    DPRINTF("calling libc open64(%s, 0x%x, 0x%x)\n", name, flags, mode);

    return __do_wrap_open(name, flags, mode, orig_open64);
}

void _init(void)
{
    fprintf(stderr, "info: Replacing open(2)!\n");
    orig_open = dlsym(RTLD_NEXT, "open");
    if (!orig_open) {
        fprintf(stderr, "error: missing symbol open!\n");
        exit(1);
    }
    orig_open64 = dlsym(RTLD_NEXT, "open64");
    if (!orig_open64) {
        fprintf(stderr, "error: missing symbol open64!\n");
        exit(1);
    }
}
