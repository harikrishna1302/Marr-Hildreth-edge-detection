# Marr-Hildreth-edge-detection
The main objective is to find the edge detection for the given input images using Marr-Hildreth process.
Before that, recall Marr-Hildreth arguments:
* Intensity changes NOT independent of image scale. Need to use operators of different sizes.
* Sudden intensity change -> peak/trough in 1st derivative.<br />
So, operator should be careful of being tuned at desired scale.

## Algorithm
* Filter the input image using NxN Gaussian filter.
* Compute Laplacian using 3x3 mask.
* Find zero crossing to the above resultant image.
## Technologies
* VisualStudio.
* OpenCV library.
* C++ programming.<br />
Make sure, the images, output and filters directories should be stored in the same project file directory. Also, all the three directories names should be given in command line arguments in visualstudio. <br />
./images ./filters ./output
## Implementation
