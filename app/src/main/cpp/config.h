#ifndef SRP_PROJECT_CONFIG_H
#define SRP_PROJECT_CONFIG_H

#include <sox.h>
#include <sys/types.h>

typedef unsigned long long uint64;

#define DEFAULT_RATE 44100

#define DEFAULT_LENGTH DEFAULT_RATE * 60

#define DEFAULT_PRECISION 16

#define DEFAULT_CHANNEL 1

#define DEFAULT_FILETYPE "raw"

#define DEFAULT_SIGNAL_INFO {DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_PRECISION, 0, NULL}

#define DEFAULT_ENCODING_INFO {SOX_ENCODING_SIGN2, 16, 1 / 0.0, sox_option_no, sox_option_no, sox_option_no, sox_false};

#endif
