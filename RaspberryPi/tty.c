/**
 * tty.c - Everything that interacts with the raw /dev/tty*
 *
 * Sources of code:
 * - man pages
 * - https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
 * - https://en.wikibooks.org/wiki/Serial_Programming/termios
 */
#include <stdio.h>  // For printf
#include <stdlib.h> // For exit
#include <string.h>

// for dirent
#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>  // write, read, close
#include <fcntl.h>   // for O_RWDR etc
#include <termios.h> // for terminal attributes
// #include <sys/ioctl.h> // ioctl for exclusive access

#include <sys/select.h>
#include <time.h>

#include "common.h"

/**
 * findAndOpenTTYUSB finds the first available ttyUSB in /dev
 * @returns int file descriptor to first /dev/ttyUSB*
 */
int findAndOpenTTYUSB(void)
{
    /**
     * Find first available ttyUSB*
     */
    DIR *dev_dp = opendir("/dev/");
    if (dev_dp == NULL)
    {
        printErrno(__func__, "Could not open /dev");
        return -1;
    }

    struct dirent *ep;
    int ttyNum = -1;
    while ((ep = readdir(dev_dp)))
    {
        sscanf(ep->d_name, "ttyUSB%d", &ttyNum);
        if (ttyNum >= 0)
            break;
    }
    if (ttyNum == -1)
    {
        printErrno(__func__, "Could not find a suitable TTYUSB*");
        return -1;
    }
    // Construct complete path
    char ttyPath[16]; // Plenty of enough room
    sprintf(ttyPath, "/dev/ttyUSB%d", ttyNum);
    printLog(__func__, "Using %s", ttyPath);

    // Now let's try to open it
    int ttyfd;
    ttyfd = open(ttyPath, O_RDONLY | O_NOCTTY); //| O_NDELAY);
    if (ttyfd == -1)
    {
        printErrno(__func__, "Could not fopen TTY!");
        return -1;
    }

    return ttyfd;
}

/**
 * prepareTTY prepares the given TTY file descriptor for serial terminal.
 * @returns status, -1 on error, ttyfd back on success
 *
 * Working TTY settings:
speed 115200 baud; rows 0; columns 0; line = 0;
intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>; eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R; werase = ^W; lnext = ^V; discard = ^O; min = 100; time = 2;
-parenb -parodd -cmspar cs8 -hupcl -cstopb cread clocal -crtscts
-ignbrk brkint ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl ixon -ixoff -iuclc -ixany -imaxbel -iutf8
-opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
-isig -icanon iexten -echo echoe echok -echonl -noflsh -xcase -tostop -echoprt echoctl echoke -flusho -extproc
 */
int setupTTY(int ttyfd)
{
    printLog(__func__, "Setting up terminal %d", ttyfd);
    if (!isatty(ttyfd))
    {
        printErrno(__func__, "Given ttyfd is not a TTY!");
        return -1;
    }

    struct termios config;
    // fetch current termios config
    if (tcgetattr(ttyfd, &config) == -1)
    {
        // erno is set
        printErrno(__func__, "Failed to get terminal interface config");
        return -1;
    }

    /**
     * Input flags - Turn off input processing
     */
    config.c_iflag &= ~(IGNBRK | ICRNL | IGNCR | INLCR | PARMRK | INPCK | ISTRIP | IXOFF | IUCLC | IXANY | IMAXBEL | IUTF8);
    config.c_iflag = (BRKINT | IGNPAR | IXON);

    /**
     * Output flags - Turn off output processing
     */
    // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OLCUC | OPOST);
    // can probably do
    config.c_oflag = 0;

    /**
     * Line flags - Turn off line processing
     */
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG);
    config.c_lflag |= (ECHOE | ECHOK | IEXTEN | ICANON);

    /**
     * Turn off character processing
     *
     * - clear current char size mask
     * - no parity checking,
     * - no output processing
     * - force 8 bit input
     */
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;

    /**
     * Ine input byte is enough to return from read()
     * Inter-character timer off
     */
    // config.c_cc[VMIN] = 100; // Minimum of characters
    // config.c_cc[VTIME] = 2; //

    /**
     * Communication speed
     */
    if (cfsetospeed(&config, B115200) == -1)
    {
        printErrno(__func__, "Couldn't set output speed of TTY");
        return -1;
    }
    // set ispeed to 0, which matches the ospeed
    if (cfsetispeed(&config, B115200) == -1)
    {
        printErrno(__func__, "Couldn't set input speed of TTY");
        return -1;
    }

    // Apply the configuration
    if (tcsetattr(ttyfd, TCSANOW, &config) == -1)
    {
        printErrno(__func__, "Couldn't set TTYconfig");
        return -1;
    }

    // Setup exclusive access to terminal
    // if (ioctl(ttyfd, TIOCEXCL, NULL) == -1)
    // {
    // printErrno(__func__, "Couldn't set TTYconfig");
    // return -1;
    // }

    printLog(__func__, "Successfully setup TTY");
    return ttyfd;
}

/**
 * closeTTY closes the given TTY FD
 * @returns whatever close() returns on the fd
 */
int closeTTY(int ttyfd)
{
    return close(ttyfd);
}

/**
 * readTTY reads whatever comes into the TTY and updates the given buffer up to bufferlength
 * @returns (negative) error code if any or (positive) data length
 */
int readTTY(int ttyfd, char *buffer, size_t bufferlength)
{

    struct timeval tv =
        {
            .tv_sec = 2,
        };
    // Init FD set
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(ttyfd, &fds);

    int ret = select(ttyfd + 1, &fds, NULL, NULL, &tv);
    if (ret < 0)
    {
        printErrno(__func__, "select failed");
        return ret;
    }
    if (ret == 0)
    {
        printLog(__func__, "Timeout");
        return ret;
    }

    int n = read(ttyfd, buffer, bufferlength);
    // Set 0 to last character to remove the '\n'
    buffer[n - 2] = 0;
    return n - 1;
}