[![Build Status](https://travis-ci.org/william-silversmith/countless.svg?branch=master)](https://travis-ci.org/william-silversmith/countless)

# countless
Code and performance tests to demonstrate the COUNTLESS algorithm, a method for downsampling image labels based on taking the mode of small image patches to perform 2x2 and 2x2x2 downsampling. This algorithm works very well with vectorized instructions. It was originally designed to work around the weakness of Python's looping speed by taking advantage of numpy operators.  

COUNTLESS 2D: Downsample an image 2x2  
COUNTLESS 3D: Downsample an image 2x2x2  
COUNTLESS N: Downsample by any factor. Watch your memory consumption.  


Read more:  
COUNTLESS 2D: https://medium.com/@willsilversmith/countless-high-performance-2x-downsampling-of-labeled-images-using-numpy-e70ad3275589  
COUNTLESS 2D Inflated: https://medium.com/@willsilversmith/countless-2d-inflated-2x-downsampling-of-labeled-images-holding-zero-values-as-background-4d13a7675f2d  
COUNTLESS 3D: https://medium.com/@willsilversmith/countless-3d-vectorized-2x-downsampling-of-labeled-volume-images-using-python-and-numpy-59d686c2f75


## Acknowledgments

Several people contributed helpful advice and assistance in developing COUNTLESS. Chris Jordan provided the seed of the C implementation of counting and countless. Dr. George Nagy suggested testing countless_if and testing the performance differential on homogenous and non-homogenous images. Jeremy Maitin-Shepard at Google originally developed the Python code for striding and downsample_with_averaging for use with neuroglancer. Special thanks to Seung Lab for providing neural segmentation labels.
