import numpy as np
from PIL import Image
import sys
import os

filename = sys.argv[1]

img = Image.open(filename)
arr = np.frombuffer(img.tobytes(), dtype=np.uint8)
arr = np.copy(arr)
# arr &= 0x00ffffff

arr = arr.reshape((img.height, img.width)).astype(np.uint8)

name, ext = os.path.splitext(filename)

name += '.raw'

with open(name, 'wb') as f:
  f.write(arr.tobytes())

print 'done'