#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <errno.h>
#include <libdevice.h>
#include <libdevinfo.h>

static int
proces_node(di_node_t d, di_minor_t dm, void *arg)
{

    char *name;

    if ((name = di_devfs_path(d)) != NULL) {

            char *dname = di_driver_name(d);

            if ((strcmp(dname, "scsi_vhci") == 0)  &&
                (strstr(name, "iport") == NULL)) {
                    printf("%s%d : /devices%s:devctl\n", dname, di_instance(d),
                       name);
            }
    }

}
void
list_vhci()
{
    di_node_t d;

    d = di_init("/", DINFOSUBTREE | DINFOMINOR);
    assert (d != NULL);

    di_walk_minor(d, DDI_NT_NEXUS, NULL, 0, &proces_node);
    di_fini(d);

    exit(0);
}

int main(int argc, char *argv[])
{

    int i = 0;
    int fd;
    struct devctl_iocdata iocdata;
    int retinfo;
    int cmd = DEVCTL_DEVICE_RESET;
    char *chop, *nodename, *unit, *minor;
    char device_copy[MAXPATHLEN];

    if (argc < 3) {
        printf("need scci_vhci MP driver and disk path as argument\n");
        list_vhci();
        return (-1);
    }

    strcpy(device_copy, argv[2]);

    /**
     * @cond
     * Device is passed in full as '/devices/scsi_vhci/disk@g600144f008002719869b5776454d0010:a,raw',
     * first locate the final '/' and null it out, then make nodename point to the first character
     * after the chop, so nodename points to 'disk@g600144f008002719869b5776454d0010:a,raw'
     * @endcond
     */
    if((chop = strrchr(device_copy, '/')) == NULL) {
        printf("Missing separator '/' in device address '%s' - \n", device_copy);
        return (-1);
    }

    *chop = '\0';
    nodename = chop + 1;
	
    /**
     * @cond
     * Now make unit point to the '@' sign after the device tree node name and
     * minor point to the minor device separator.
     * @endcond
     */
     unit = strchr(nodename, '@');
     minor = strchr(nodename, ':' );

    /**
     * @cond
     * Terminate at the unit; this makes nodename now point
     * to the 'disk' part.
     * Increment unit so it point to the unit part immediately
     * after the nodename.
     * @endcond
     */
    if( unit != NULL ) {
        *unit++ = '\0';
    } else {
        printf("Missing separator '@' in device address '%s'\n", device_copy);
        return (-1);
    }

    if( minor != NULL ) {
        *minor = '\0';
    }

    printf("nodename '%s' unit addr is '%s'\n", nodename, unit);

    if ((fd = open(argv[1], O_RDWR)) == -1) {
        printf("Failed to open %s : %s", argv[1], strerror(errno));
        return (-1);
    }

    (void) memset(&iocdata, 0, sizeof (struct devctl_iocdata));

    iocdata.cmd = cmd;
    iocdata.c_nodename = nodename;
    iocdata.c_unitaddr = unit;
    iocdata.cpyout_buf = &retinfo;

    printf("Disk reset success...");
    if (ioctl(fd, cmd, &iocdata) == -1) {
        printf("failed with - (%d) %s \n", retinfo, strerror(errno));
        return (-1);
    }
    printf("ok resulted in - (%d)\n", retinfo);

    if (fd >0)
        close(fd);
    return (0);
}
