#include <imdecoder/imdecoder.h>
#include <iostream>

int main() {
  // Create an instance of the Imdecoder class
  imd::Imdecoder imd;

  // Load an image file
  imd::Image image = imd.load("input.jpg");

  // Resize the image
  image.resize(640, 480);

  // Save the resized image to a new file
  imd.save("output.jpg", image);

  // Print a message to indicate that the operation is complete
  std::cout << "Image processing complete!" << std::endl;

  return 0;
}

