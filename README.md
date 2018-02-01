[![Build Status](https://travis-ci.org/william-silversmith/countless.svg?branch=master)](https://travis-ci.org/william-silversmith/countless)

# countless
Code and performance tests to demonstrate the COUNTLESS algorithm, a method for downsampling image labels based on taking the mode of 2x2 blocks.

Read more about this
https://medium.com/@willsilversmith/countless-high-performance-2x-downsampling-of-labeled-images-using-numpy-e70ad3275589


## Acknowledgments

Several people contributed helpful advice and assistance in developing COUNTLESS. Chris Jordan provided the seed of the C implementation of counting and countless. Dr. George Nagy suggested testing countless_if and testing the performance differential on homogenous and non-homogenous images. Jeremy Maitin-Shepard at Google originally developed the Python code for striding and downsample_with_averaging for use with neuroglancer. Special thanks to Seung Lab for providing neural segmentation labels.