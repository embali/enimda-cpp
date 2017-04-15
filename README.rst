ENIMDA
======

Entropy-based image border detection algorithm: detect border or whitespace offset for every side of image,
supports animated GIFs.

Prerequisites
-------------

GraphicsMagick (http://www.graphicsmagick.org/)

Algorithm (simplified)
----------------------

For each side of the image starting from top, rotating image counterclockwise to keep side of interest on top:

* Get upper block 25% of image height
* Get lower block with the same height as the upper one
* Calculate entropy for both blocks and their difference
* Make upper block 1px less
* Repeat from p.2 until we hit image edge
* Border is between blocks with entropy difference maximum

.. image:: https://raw.githubusercontent.com/embali/enimda-rs/master/algorithm.gif
    :alt: Sliding from center to edge - searching for maximum entropy difference
    :width: 300
    :height: 300

Example
-------

Find image borders:

.. code-block:: c++

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

Demo
----

For demo please refer to `ENIMDA Demo <https://github.com/embali/enimda-demo/>`_

Also it lives at `Picture Instruments <http://picinst.com/>`_ as 'Remove borders' instrument
