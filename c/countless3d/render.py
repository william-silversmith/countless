import numpy as np
from PIL import Image

import os

if not os.path.exists('./results'):
  os.makedirs('./results')


with open('output.raw', 'rb') as f:
    buf = f.read()

seg = np.frombuffer(buf, dtype=np.uint16).reshape( (512, 512, 32) ).T
print(seg.shape)
for z in range(seg.shape[0]):
     img2d = seg[z,:,:]
     img2d = img2d.astype(np.uint32)
     img2d[:,:] |= 0xff000000
     img2d = Image.fromarray(img2d, 'RGBA')
     img2d.save('./results/' + str(z) + '.png', 'PNG')
