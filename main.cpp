#include "KeyhoteeApplication.hpp"

#ifdef ALPHA_RELEASE
bool gMiningIsPossible = false;
#else
bool gMiningIsPossible = true;
#endif /// ALPHA_RELEASE


int main(int argc, char** argv)
  {
  return TKeyhoteeApplication::run(argc, argv);
  }

