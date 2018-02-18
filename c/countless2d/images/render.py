from PIL import Image
import numpy as np

f = open('countless_output.bin', 'rb')
raw = np.frombuffer(f.read(), dtype=np.uint8)
# reshape = raw.reshape( (3632 / 2, 5456 / 2) )
reshape = raw.reshape( (512, 512) )
img = Image.fromarray(reshape)
img.save('output.png', 'PNG', mode='L')
