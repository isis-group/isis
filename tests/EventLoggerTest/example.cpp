
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include <iostream>

struct FileIO{
  enum {use_rel = _ENABLE_FILEIO_LOG};
};

int main(int argc, char *argv[])
{
  ENABLE_LOG(FileIO,MessagePrintNeq,2);
  {
    MAKE_LOG(FileIO);
    LOG(FileIO,2) << "Hallo " << MSubject("test") <<  std::endl;
    LOG(FileIO,2) << "Hallo " << MSubject("test") <<  std::endl;
    LOG(FileIO,2) << "Hallo2 " << MSubject("test2") <<  std::endl;
    LOG(FileIO,2) << "Hallo " << MSubject("test") <<  std::endl;
  }

  return EXIT_SUCCESS;
}
