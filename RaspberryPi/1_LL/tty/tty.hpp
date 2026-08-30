#ifndef TTY_H
#define TTY_H

#include "lib/error_codes.h"
#include <memory>
#include <span>
#include <optional>

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

class LineReader {
public:
    LineReader() = delete;    
    explicit LineReader(Tty &tty);

    std::string read(void);

private:
    Tty &m_tty;

    std::string m_buffer;
};

std::unique_ptr<Tty> findAndOpenTTYUSB(void);
error_e setupTTY(Tty*);
int readTTY(Tty*, std::span<char> data);

#endif