C Downsampling
==============

You'll need to change the X_DIM and Y_DIM of each file to match the image you'll be downsampling.

gray_segmentation.raw: 1024 (x) x 1024 (y)
gcim.raw: 5456 (x) x 3632 (y)


Compile each file using whatever tricks your compiler will afford. Example using clang:  

`clang -Ofast counting.c -o counting`  

Example using GCC:  

`gcc -O3 counting.c -o counting`


## Running 

`./counting images/gcim.raw`  

## Output

Each run will output a file called `countless_output.bin`. You can see the rendered image using the python file render.py as a guide which will change it into a png.

## Creating New Files

You can create new .raw files from images of your choosing using toraw.py as a guide.

