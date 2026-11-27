/* Restrict this test process and its children to the stage's runtime resources. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/landlock.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static const uint64_t read_data = LANDLOCK_ACCESS_FS_READ_FILE | LANDLOCK_ACCESS_FS_READ_DIR;

static int allow(int ruleset, const char *path, uint64_t rights, int required)
{
    struct stat st;
    int fd = open(path, O_PATH | O_CLOEXEC);
    if (fd == -1)
        return !required && errno == ENOENT ? 0 : -1;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return -1;
    }
    if (!S_ISDIR(st.st_mode))
        rights &= LANDLOCK_ACCESS_FS_READ_FILE | LANDLOCK_ACCESS_FS_WRITE_FILE
                  | LANDLOCK_ACCESS_FS_EXECUTE;
    struct landlock_path_beneath_attr rule = {
        .parent_fd = fd,
        .allowed_access = rights,
    };
    int result = syscall(SYS_landlock_add_rule, ruleset, LANDLOCK_RULE_PATH_BENEATH, &rule, 0);
    close(fd);
    return result;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fputs("Usage: native-profile-sandbox RUNTIME_ROOT PROGRAM [ARG ...]\n", stderr);
        return 2;
    }

    struct landlock_ruleset_attr attr = {
        .handled_access_fs = read_data | LANDLOCK_ACCESS_FS_EXECUTE |
            LANDLOCK_ACCESS_FS_WRITE_FILE | LANDLOCK_ACCESS_FS_REMOVE_DIR |
            LANDLOCK_ACCESS_FS_REMOVE_FILE | LANDLOCK_ACCESS_FS_MAKE_DIR |
            LANDLOCK_ACCESS_FS_MAKE_REG | LANDLOCK_ACCESS_FS_MAKE_SYM |
            LANDLOCK_ACCESS_FS_MAKE_CHAR | LANDLOCK_ACCESS_FS_MAKE_BLOCK |
            LANDLOCK_ACCESS_FS_MAKE_SOCK | LANDLOCK_ACCESS_FS_MAKE_FIFO,
    };
    int ruleset = syscall(SYS_landlock_create_ruleset, &attr, sizeof(attr), 0);
    const char *data[] = { "/etc", "/proc", "/sys", "/usr/share/X11",
                           "/usr/share/fonts", "/usr/share/fontconfig",
                           "/usr/share/alsa", "/usr/share/locale",
                           "/usr/share/zoneinfo", "/var/cache/fontconfig",
                           "/tmp/.X11-unix" };
    const char *devices[] = { "/dev/null", "/dev/urandom", "/dev/random" };
    if (ruleset < 0 || allow(ruleset, argv[1], attr.handled_access_fs, 1) < 0)
        goto fail;
    for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
        if (allow(ruleset, data[i], read_data, 0) < 0)
            goto fail;
    }
    for (size_t i = 0; i < sizeof(devices) / sizeof(devices[0]); ++i) {
        if (allow(ruleset, devices[i], read_data | LANDLOCK_ACCESS_FS_WRITE_FILE, 0) < 0)
            goto fail;
    }
    const char *runtime = getenv("PIPEWIRE_RUNTIME_DIR");
    if (runtime && allow(ruleset, runtime, read_data, 1) < 0)
        goto fail;
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0 ||
        syscall(SYS_landlock_restrict_self, ruleset, 0) < 0)
        goto fail;
    close(ruleset);
    execv(argv[2], &argv[2]);
    perror(argv[2]);
    return 1;

fail:
    perror("stage runtime restriction");
    return 1;
}
