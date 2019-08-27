#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/scsi/scsi_types.h>

int
uscsi_reset(int disk_fd)
{
    struct scsi_inquiry inq;
    struct scsi_extended_sense sense;
    struct uscsi_cmd ucmd;
    union scsi_cdb cdb;
    int rc;

    (void) memset(&inq, 0, sizeof (struct scsi_inquiry));
    (void) memset(&sense, 0, sizeof (struct scsi_extended_sense));
    (void) memset(&ucmd, 0, sizeof (ucmd));
    (void) memset(&cdb, 0, sizeof (union scsi_cdb));

    cdb.scc_cmd = SCMD_INQUIRY;
    /* bytes 3-4 contain data-in buf len */
    cdb.cdb_opaque[3] = (uint8_t)((sizeof (struct scsi_inquiry) >> 8) &
        0xff);
    cdb.cdb_opaque[4] = (uint8_t)((sizeof (struct scsi_inquiry)) & 0xff);

    ucmd.uscsi_cdb = (caddr_t)&cdb;
    ucmd.uscsi_cdblen = CDB_GROUP0;
    ucmd.uscsi_flags = USCSI_RESET_LUN | USCSI_READ;
    ucmd.uscsi_bufaddr = (caddr_t)&inq;
    ucmd.uscsi_buflen = sizeof (struct scsi_inquiry);
    ucmd.uscsi_timeout = 30;
    ucmd.uscsi_rqbuf = (caddr_t)&sense;
    ucmd.uscsi_rqlen = sizeof (struct scsi_extended_sense);
    ucmd.uscsi_rqstatus = 0xff;

    rc = ioctl(disk_fd, USCSICMD, &ucmd);
    if (rc == 0) {
        puts("Reset successful!");
    } else {
        printf("Reset failed: errno = %d\n", errno);
    }

    return rc;
}

int main(int argc, const char **argv)
{
    int fd;

    if (argc < 2) {
        puts("Provide path to the device");
        return 1;
    }

    fd = open(argv[1], O_RDWR | O_NDELAY);
    if (fd == -1) {
        printf("Failed to open the device: errno = %d\n", errno);
        return 1;
    }

    return uscsi_reset(fd);
}

