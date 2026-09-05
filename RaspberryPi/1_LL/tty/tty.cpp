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
#include <functional>

// #include <sys/ioctl.h> // ioctl for exclusive access
#include <sys/types.h>
#include <sys/select.h>
#include <dirent.h>
#include <time.h>

#include <debug.h>
#include "tty.hpp"

/** Length of DSMR line */
constexpr size_t c_DsmrLineLength = 512U;

LineReader::LineReader(Tty &tty) : m_tty(tty) {};

/**
 * @brief keeps reading from the TTY, buffers excess data, extracts a line 
 */
std::string LineReader::read(void)
{
    std::string line{};

    while (1) {
        line.clear();
        line.resize(c_DsmrLineLength);

        const int readBytes = readTTY(&m_tty, std::span{line.data(), line.size()});
        if (readBytes < 0)
        {
            DBG_ERR( "readTTY returned a fatal response!");
            return "";
        }
    
        /* Resize string to exact number of received bytes */
        line.resize(readBytes);
        m_buffer += line;

        if (m_buffer.contains('\n'))
        {
            /* Extract line */
            const size_t newlineIndex = m_buffer.find('\n');
            const std::string totalLine = m_buffer.substr(0, newlineIndex);

            /* Remove the line and the newline */
            m_buffer.erase(0, newlineIndex + 1);

            return totalLine;
        }
    }
}

/**
 * findAndOpenTTYUSB finds the first available ttyUSB in /dev
 * @returns int file descriptor to first /dev/ttyUSB*
 */
std::unique_ptr<Tty> findAndOpenTTYUSB(void)
{
    /* Find first available ttyUSB* */
    auto dir = std::unique_ptr<DIR, std::function<void(DIR *)>>(opendir("/dev/"), [](DIR *d)
                                                                { if (d) {closedir(d);} });
    if (!dir)
    {
        DBG_ERR("Could not open /dev");
        return nullptr;
    }

    struct dirent *ep;
    int ttyNum = -1;
    while ((ep = readdir(dir.get())))
    {
        sscanf(ep->d_name, "ttyUSB%d", &ttyNum);
        if (ttyNum >= 0)
            break;
    }

    if (ttyNum < 0)
    {
        DBG_ERR("Could not find a suitable TTYUSB");
        return nullptr;
    }
    // Construct complete path
    char ttyPath[32];
    sprintf(ttyPath, "/dev/ttyUSB%d", ttyNum);
    DBG_INFO("Using %s", ttyPath);

    return (std::make_unique<Tty>(ttyPath));
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
error_e setupTTY(Tty *tty)
{
    DBG_INFO("Setting up terminal %d", tty->getFd());
    if (!isatty(tty->getFd()))
    {
        DBG_ERR("Given ttyfd is not a TTY!");
        return eError_failed;
    }

    struct termios config;
    // fetch current termios config
    if (tcgetattr(tty->getFd(), &config) == -1)
    {
        // erno is set
        DBG_ERR("Failed to get terminal interface config");
        return eError_failed;
    }

    /**
     * Input flags - Turn off input processing
     */
    config.c_iflag &= ~(IGNBRK | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXOFF | IUCLC | IXANY | IMAXBEL | IUTF8);
    config.c_iflag = (BRKINT | IGNPAR | IXON | IGNCR);

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
        DBG_ERR("Couldn't set output speed of TTY");
        return eError_failed;
    }
    // set ispeed to 0, which matches the ospeed
    if (cfsetispeed(&config, B115200) == -1)
    {
        DBG_ERR("Couldn't set input speed of TTY");
        return eError_failed;
    }

    // Apply the configuration
    if (tcsetattr(tty->getFd(), TCSANOW, &config) == -1)
    {
        DBG_ERR( "Couldn't set TTYconfig");
        return eError_failed;
    }

    // Setup exclusive access to terminal
    // if (ioctl(ttyfd, TIOCEXCL, NULL) == -1)
    // {
    // printErrno(__func__, "Couldn't set TTYconfig");
    // return -1;
    // }

    DBG_INFO("Successfully setup TTY");
    return eError_ok;
}

/**
 * readTTY reads whatever comes into the TTY and updates the given buffer up to bufferlength
 * @returns (negative) error code if any or (positive) data length
 */
int readTTY(Tty *tty, std::span<char> data)
{

    struct timeval tv =
        {
            .tv_sec = 2,
            .tv_usec = 0,
        };
    // Init FD set
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(tty->getFd(), &fds);

    int ret = select(tty->getFd() + 1, &fds, NULL, NULL, &tv);
    if (ret < 0)
    {
        DBG_ERR("select failed");
        return ret;
    }
    if (ret == 0)
    {
        DBG_ERR("Timeout");
        return ret;
    }

    return read(tty->getFd(), data.data(), data.size());
}