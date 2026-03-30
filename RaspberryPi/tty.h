#ifndef TTY_H
#define TTY_H

int findAndOpenTTYUSB(void);
int setupTTY(int);
int closeTTY(int);
int readTTY(int, char *, size_t);
#endif