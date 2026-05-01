#ifndef TTY_H
#define TTY_H

#include <cstdlib>
#include <memory>
#include <span>
#include <exception>

// for dirent
#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>  // write, read, close
#include <fcntl.h>   // for O_RWDR etc
#include <termios.h> // for terminal attributes

class Tty{
public:
    explicit Tty(int fd): m_fd{fd} {}
    explicit Tty(const char *path) {
        m_fd = open(path, O_RDONLY |O_NOCTTY);
        if (m_fd == -1) {
            throw;
        }
    }

    /* Destructor */
    ~Tty() {
        if (m_fd != -1) {
            close(m_fd);
        }
    }

    /* Remove copy constructor */
    Tty(const Tty &) = delete;

    /* Remove copy assignment */
    Tty &operator=(const Tty&) = delete;

    /* Get FD */
    constexpr int getFd(void) {
        return m_fd;
    }

private:
    int m_fd {-1};
};

std::unique_ptr<Tty> findAndOpenTTYUSB(void);
int setupTTY(Tty*);
int readTTY(Tty*, std::span<char> data);

#endif