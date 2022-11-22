#define MAIN_H_
#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>


int create_container(void);
int child(void *args);
void mount_partition(void);
void create_partition(void);
void pull_alpine(void);

int main(void)
{
    int namespaces = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWNET;
    pid_t container;

    container = clone(child, malloc(4096) + 4096, namespaces | SIGCHLD, NULL);
    waitpid(container, NULL, 0);



    return EXIT_SUCCESS;
}

int create_container(void)
{
    char *cmd[] = { "/bin/bash", NULL };
    execv("/bin/bash", cmd);
    exit(EXIT_FAILURE);

    system("mount -t proc proc /proc");

}

int child(void *args)
{
    (void)args;

    create_partition();
    mount_partition();
    pull_alpine();


    create_container();
    return (0);
}

void create_partition(void) {
    char cmd[1024];

    // create a disk
    system("dd if=/dev/zero of=disk12.img bs=512 count=4194304");

    // format as ext4
    snprintf(cmd, 1024, "mkfs.ext4 ./disk12.img");
    system(cmd);
}


void mount_partition(void) {
    char cmd[1024];

    snprintf(cmd, 1024, "losetup -fP disk12.img");
    system(cmd);

    system("mkdir mnt");

    snprintf(cmd, 1024, "mount -o loop /dev/loop12 mnt");
    system(cmd);

    chdir("mnt");
}

void pull_alpine(void) {
    system("mkdir temp");

    system("docker export $(docker create alpine) | tar -C temp -xvf -");
    system("cp -r temp/. mnt/alpine");

    system("rm -rf temp");

        system("mount --make-rprivate /");
    system("mount --bind mnt/alpine mnt/alpine");

    system("mkdir mnt/alpine/oldrootfs");
    syscall(SYS_pivot_root, "mnt/alpine", "mnt/alpine/oldrootfs");
    chdir("/");
}
