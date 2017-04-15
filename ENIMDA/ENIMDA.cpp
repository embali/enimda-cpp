//
// Copyright Anton Smolin 2016-2017
//
// ENIMDA implementation
//

#include "ENIMDA.hpp"
#include <Magick++.h>
#include <cmath>
#include <ctime>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace Magick;

namespace ENIMDA {
Side::Side(vector<vector<unsigned char>> &pixels) : pixels(move(pixels)) {
  this->height = this->pixels.size();
  this->width = (this->height > 0) ? this->pixels.front().size() : 0;
}

Side::~Side() { this->pixels.clear(); }

float Side::entropy(unsigned int row, unsigned int height) {
  unordered_map<unsigned char, unsigned int> counts;

  for (unsigned int h = row; h < row + height; ++h) {
    for (unsigned int w = 0; w < this->width; ++w) {
      unsigned char pixel = this->pixels[h][w];

      if (counts.count(pixel)) {
        ++counts[pixel];
      } else {
        counts[pixel] = 1;
      }
    }
  }

  float entropy = 0.0;
  float length = height * this->width;
  for (auto &count : counts) {
    float probability = (float)count.second / length;
    entropy -= probability * log2(probability);
  }

  return entropy;
}

unsigned int Side::scan(bool deep, float threshold) {
  unsigned int border = 0;
  unsigned int rows = this->height / 2;

  while (true) {
    unsigned int sub = 0;
    float delta = threshold;

    for (unsigned int center = rows; center > border; --center) {
      float upper = this->entropy(border, center - border);
      float lower = this->entropy(center, center - border);
      float diff = (lower > 0.0) ? upper / lower : delta;

      if (diff < delta && diff < threshold) {
        delta = diff;
        sub = center;
      }
    }

    if (sub == 0 || border == sub) {
      break;
    }

    border = sub;

    if (!deep) {
      break;
    }
  }

  return border;
}

vector<unsigned int> Slicer::slice(unsigned int count, unsigned int limit) {
  vector<unsigned int> strips(count);
  iota(strips.begin(), strips.end(), 0);

  if (limit > 0 && limit < count) {
    random_shuffle(strips.begin(), strips.end());
    strips.resize(limit);
  }
  sort(strips.begin(), strips.end());

  return strips;
}

Frame::Frame(Image &image, float depth, unsigned int columns) {
  Image moved = move(image);
  Pixels pixel(moved);
  const PixelPacket *single = pixel.getConst(0, 0, 1, 1);
  float divider = pow(2, (sizeof(single->red) - 1) * 8);

  for (unsigned int side = 0; side < 4; ++side) {
    if (side) {
      moved.rotate(-90.0);
    }
    unsigned int width = moved.columns();
    unsigned int height = 2 * depth * moved.rows();
    Pixels view(moved);
    const PixelPacket *pixels = view.getConst(0, 0, width, height);

    vector<unsigned int> strips = this->slice(width, columns);
    unordered_set<unsigned int> columnset;
    for (auto &strip : strips) {
      columnset.insert(strip);
    }

    vector<vector<unsigned char>> rows;
    for (unsigned int r = 0; r < height; ++r) {
      vector<unsigned char> row;

      for (unsigned int c = 0; c < width; ++c) {
        if (columnset.count(c)) {
          row.push_back(round(pixels->red / divider));
        }
        ++pixels;
      }

      rows.push_back(row);
    }
    this->sides.push_back(Side(rows));
  }
}

Frame::~Frame() { this->sides.clear(); }

vector<unsigned int> Frame::scan(bool deep, float threshold) {
  vector<unsigned int> borders;

  for (auto &side : this->sides) {
    borders.push_back(side.scan(deep, threshold));
  }

  return borders;
}

Loader::Loader(const string &path, unsigned int frames, unsigned int resize,
               float depth, unsigned int columns) {
  srand(unsigned(time(0)));

  vector<Image> images;
  readImages(&images, path);
  unsigned int framecount = images.size();

  vector<Image> converted;
  if (framecount > 1) {
    coalesceImages(&converted, images.begin(), images.end());
    images.clear();
  } else {
    converted.swap(images);
  }

  int delta = framecount - frames;
  if (frames && delta > 0) {
    vector<unsigned int> strips = this->slice(framecount, (unsigned int)delta);
    reverse(strips.begin(), strips.end());
    for (auto &strip : strips) {
      converted.erase(converted.begin() + strip);
    }
  }

  unsigned int width = converted.front().columns();
  unsigned int height = converted.front().rows();
  this->multiplier = 1.0;
  for (auto &image : converted) {
    if (resize > 0) {
      image.resize(Geometry(resize, resize));
      if (&image == &converted.front()) {
        unsigned int c = image.columns();
        unsigned int r = image.rows();
        this->multiplier =
            (c > r) ? (float)width / (float)c : (float)height / (float)r;
      }
    }
    image.type(GrayscaleType);

    this->frames.push_back(Frame(image, depth, columns));
  }
}

Loader::~Loader() { this->frames.clear(); }

vector<unsigned int> Loader::scan(bool deep, bool outer, float threshold) {
  vector<unsigned int> borders = {0, 0, 0, 0};

  for (auto &frame : this->frames) {
    vector<unsigned int> sub = frame.scan(deep, threshold);

    for (unsigned int side = 0; side < 4; ++side) {
      if ((outer && (sub[side] < borders[side] || !borders[side])) ||
          (!outer && sub[side] > borders[side])) {
        borders[side] = sub[side];
      }
    }
  }

  for (auto &border : borders) {
    border *= this->multiplier;
  }

  return borders;
}
}
