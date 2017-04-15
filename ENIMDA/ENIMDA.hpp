//
// Copyright Anton Smolin 2016-2017
//
// ENIMDA definition
//

#ifndef ENIMDA_Header
#define ENIMDA_Header

#include <Magick++.h>
#include <vector>

namespace ENIMDA {
// Rotated image side, which contains only required pixel information
class Side {
  friend class Frame;

private:
  unsigned int width;
  unsigned int height;
  std::vector<std::vector<unsigned char>> pixels;

  // Construct side moving pixel 2D vector
  Side(std::vector<std::vector<unsigned char>> &pixels);

  // Calculate entropy of block with height, starting from row
  float entropy(unsigned int row, unsigned int height);

  // Find side border
  unsigned int scan(bool deep, float threshold);

public:
  ~Side();
};

// Slicer produces limited random indexes slice using sequence from 0 to count
class Slicer {
protected:
  std::vector<unsigned int> slice(unsigned int count, unsigned int limit);
};

// Frame represents four sides of image frame
class Frame : private Slicer {
  friend class Loader;

private:
  std::vector<Side> sides;

  // Constructs frame moving image and using scan depth and column limit
  // arguments
  Frame(Magick::Image &image, float depth, unsigned int columns);

  // Find frame borders
  std::vector<unsigned int> scan(bool deep, float threshold);

public:
  ~Frame();
};

// Prepares image frames for scanning
class Loader : private Slicer {
private:
  std::vector<Frame> frames;
  float multiplier;

public:
  // Constructs loader with image path and limits
  // Arguments:
  //   frames - limit number of frames to process, 0 means no limit
  //   resize - resize image to fit this size, 0 means no resize
  //   depth - depth of side (e.g. 0.25 means that we use only 1/4 of side
  //   height for further scan), 0.5 is maximum depth
  //   columns - limit number of columns to process, 0 means no limit
  Loader(const std::string &path, unsigned int frames, unsigned int resize,
         float depth, unsigned int columns);

  ~Loader();

  // Find image borders
  // Arguments:
  //   deep - if set to true then scan will be recursive (find borders inside
  //   region limited with previously found borders)
  //   outer - used for animated GIFs, if set to true then border which is
  //   closer to the edge will be chosen from frames' borders
  //   threshold - algorithm aggressiveness, use 0.5 if not sure
  std::vector<unsigned int> scan(bool deep, bool outer, float threshold);
};
}
#endif
