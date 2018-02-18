COUNTLESS 3D C Implementation
=============================

This is not working correctly yet!  

To build:

```
make
```

To run:

```
lzma -d 3d.raw.lzma # Uncompress the demonstration volume
./counting 3d.raw
python render.py # to visualize results

# OR 

./countless3d 3d.raw
python render.py # to visualize results

```

The core of counting.c was contributed by Chris Jordan.