#ifdef WIN32
#include <Windows.h>
#include <wincon.h>
#endif

#include "KeyhoteeApplication.hpp"

#include <QDebug>
#include <QFile>

bool gMiningIsPossible = true;

int main(int argc, char** argv)
  {
#ifdef WIN32
  bool console_ok = AllocConsole();

  freopen("CONOUT$", "wb", stdout);
  freopen("CONOUT$", "wb", stderr);
  //freopen( "console.txt", "wb", stdout);
  //freopen( "console.txt", "wb", stderr);
  printf("testing stdout\n");
  fprintf(stderr, "testing stderr\n");
#endif
  
  int exitCode = TKeyhoteeApplication::run(argc, argv);

#ifdef WIN32
  fclose(stdout);
  FreeConsole();
#endif

  return exitCode;
  }

