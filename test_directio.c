/* Run under LD_PRELOAD=libdirectio (see `make check`). */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int __open_2(const char *, int);
int __open64_2(const char *, int);
int __openat_2(int, const char *, int);
int __openat64_2(int, const char *, int);

#define CHECK(c) do { if (!(c)) { \
	fprintf(stderr, "%s:%d: FAILED: %s\n", __FILE__, __LINE__, #c); \
	failed = 1; } } while (0)

static const char *file = "test_directio.dat", *fifo = "test_directio.fifo";
static int failed;

/* 1 if fd has O_DIRECT, 0 if not, -1 if the open failed; closes fd. */
static int direct(int fd)
{
	int r;

	if (fd < 0)
		return -1;
	r = (fcntl(fd, F_GETFL) & O_DIRECT) != 0;
	close(fd);
	return r;
}

int main(void)
{
	struct stat st;
	void *buf = NULL;
	int fd;

	umask(022);
	unlink(file);
	unlink(fifo);

	/* Regular files get O_DIRECT through every wrapped entry point. */
	CHECK(direct(open(file, O_RDWR | O_CREAT | O_EXCL, 0640)) == 1);
	CHECK(stat(file, &st) == 0 && (st.st_mode & 0777) == 0640);
	CHECK(direct(open64(file, O_RDONLY)) == 1);
	CHECK(direct(openat(AT_FDCWD, file, O_RDONLY)) == 1);
	CHECK(direct(openat64(AT_FDCWD, file, O_RDONLY)) == 1);
	CHECK(direct(creat(file, 0644)) == 1);
	CHECK(direct(creat64(file, 0644)) == 1);
	CHECK(direct(__open_2(file, O_RDONLY)) == 1);
	CHECK(direct(__open64_2(file, O_RDONLY)) == 1);
	CHECK(direct(__openat_2(AT_FDCWD, file, O_RDONLY)) == 1);
	CHECK(direct(__openat64_2(AT_FDCWD, file, O_RDONLY)) == 1);

	/* Aligned I/O goes through. */
	CHECK(posix_memalign(&buf, 4096, 4096) == 0);
	CHECK((fd = open(file, O_RDWR)) >= 0);
	CHECK(pwrite(fd, buf, 4096, 0) == 4096);
	CHECK(pread(fd, buf, 4096, 0) == 4096);
	close(fd);

	/* Everything else still opens, without O_DIRECT. */
	CHECK(direct(open("/dev/null", O_WRONLY)) == 0);
	CHECK(direct(open("/dev/zero", O_RDONLY)) == 0);
	CHECK(direct(open("/proc/self/status", O_RDONLY)) == 0);
	CHECK(direct(open(".", O_RDONLY | O_DIRECTORY)) == 0);
	CHECK(mkfifo(fifo, 0600) == 0);
	CHECK(direct(open(fifo, O_RDWR)) == 0);

	unlink(file);
	unlink(fifo);
	puts(failed ? "FAIL" : "ok");
	return failed;
}
