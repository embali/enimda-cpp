#include "ENIMDA.hpp"
#include <iostream>

using namespace std;
using namespace Magick;
using namespace ENIMDA;

int main(int argc, char **argv) {
  InitializeMagick(*argv);

  Loader em("example.jpeg", 0, 0, 0.25, 0);
  vector<unsigned int> borders = em.scan(true, true, 0.5);

  for (auto &border : borders) {
    cout << border << " ";
  }
  cout << endl;

  return 0;
}
