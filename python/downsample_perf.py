"""
COUNTLESS performance test in Python.

python downsample_perf.py ./images/NAMEOFIMAGE
"""

from collections import defaultdict
import io
import os
from PIL import Image
import math
import numpy as np
import random
import sys
import time
from tqdm import tqdm


def simplest_countless(data):
  """
  Vectorized implementation of downsampling a 2D 
  image by 2 on each side using the COUNTLESS algorithm.
  
  data is a 2D numpy array with even dimensions.
  """
  sections = []
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  factor = (2,2)
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  a, b, c, d = sections

  ab = a * (a == b) # PICK(A,B)
  ac = a * (a == c) # PICK(A,C)
  bc = b * (b == c) # PICK(B,C)

  a = ab | ac | bc # Bitwise OR, safe b/c non-matches are zeroed
  
  return a + (a == 0) * d # AB || AC || BC || D

def quick_countless(data):
  """
  Vectorized implementation of downsampling a 2D 
  image by 2 on each side using the COUNTLESS algorithm.
  
  data is a 2D numpy array with even dimensions.
  """
  sections = []
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  factor = (2,2)
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  a, b, c, d = sections

  ab_ac = a * ((a == b) | (a == c)) # PICK(A,B) || PICK(A,C) w/ optimization
  bc = b * (b == c) # PICK(B,C)

  a = ab_ac | bc # (PICK(A,B) || PICK(A,C)) or PICK(B,C)
  
  return a + (a == 0) * d # AB || AC || BC || D

def zero_corrected_countless(data):
  """
  Vectorized implementation of downsampling a 2D 
  image by 2 on each side using the COUNTLESS algorithm.
  
  data is a 2D numpy array with even dimensions.
  """
  # allows us to prevent losing 1/2 a bit of information 
  # at the top end by using a bigger type. Without this 255 is handled incorrectly.
  data, upgraded = upgrade_type(data) 

  data = data + 1 # don't use +=, it will affect the original data.

  sections = []
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  factor = (2,2)
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  a, b, c, d = sections

  ab = a * (a == b) # PICK(A,B)
  ac = a * (a == c) # PICK(A,C)
  bc = b * (b == c) # PICK(B,C)

  a = ab | ac | bc # Bitwise OR, safe b/c non-matches are zeroed
  
  result = a + (a == 0) * d - 1 # a or d - 1

  if upgraded:
    return downgrade_type(result)

  return result

def countless(data):
  """
  Vectorized implementation of downsampling a 2D 
  image by 2 on each side using the COUNTLESS algorithm.
  
  data is a 2D numpy array with even dimensions.
  """
  # allows us to prevent losing 1/2 a bit of information 
  # at the top end by using a bigger type. Without this 255 is handled incorrectly.
  data, upgraded = upgrade_type(data) 

  data = data + 1 # don't use +=, it will affect the original data.

  sections = []
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  factor = (2,2)
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  a, b, c, d = sections

  ab_ac = a * ((a == b) | (a == c)) # PICK(A,B) || PICK(A,C) w/ optimization
  bc = b * (b == c) # PICK(B,C)

  a = ab_ac | bc # (PICK(A,B) || PICK(A,C)) or PICK(B,C)
  result = a + (a == 0) * d - 1 # (matches or d) - 1

  if upgraded:
    return downgrade_type(result)

  return result

def upgrade_type(arr):
  dtype = arr.dtype

  if dtype == np.uint8:
    return arr.astype(np.uint16), True
  elif dtype == np.uint16:
    return arr.astype(np.uint32), True
  elif dtype == np.uint32:
    return arr.astype(np.uint64), True

  return arr, False
  
def downgrade_type(arr):
  dtype = arr.dtype

  if dtype == np.uint64:
    return arr.astype(np.uint32)
  elif dtype == np.uint32:
    return arr.astype(np.uint16)
  elif dtype == np.uint16:
    return arr.astype(np.uint8)
  
  return arr

def counting(array):
    factor = (2, 2, 1)
    shape = array.shape

    if len(shape) < 3:
      array = array[ :,:, np.newaxis ]
      shape = array.shape

    output_shape = tuple(int(math.ceil(s / f)) for s, f in zip(shape, factor))
    output = np.zeros(output_shape, dtype=np.uint8)

    for chan in xrange(0, shape[2]):
      for x in xrange(0, shape[0], 2):
        for y in xrange(0, shape[1], 2):
          block = array[ x:x+2, y:y+2, chan ] # 2x2 block

          hashtable = defaultdict(int)
          for subx,suby in np.ndindex(block.shape[0], block.shape[1]):
            hashtable[block[subx, suby]] += 1

          best = (0, 0)
          for segid, val in hashtable.iteritems():
            if best[1] < val:
              best = (segid, val)

          output[ x / 2, y / 2, chan ] = best[0]
    
    return np.squeeze(output)

def countless_if(array):
    factor = (2, 2, 1)
    shape = array.shape

    if len(shape) < 3:
      array = array[ :,:, np.newaxis ]
      shape = array.shape

    output_shape = tuple(int(math.ceil(s / f)) for s, f in zip(shape, factor))
    output = np.zeros(output_shape, dtype=np.uint8)

    for chan in xrange(0, shape[2]):
      for x in xrange(0, shape[0], 2):
        for y in xrange(0, shape[1], 2):
          block = array[ x:x+2, y:y+2, chan ] # 2x2 block

          if block[0,0] == block[1,0]:
            pick = block[0,0]
          elif block[0,0] == block[0,1]:
            pick = block[0,0]
          elif block[1,0] == block[0,1]:
            pick = block[1,0]
          else:
            pick = block[1,1]

          output[ x / 2, y / 2, chan ] = pick
    
    return np.squeeze(output)

def downsample_with_averaging(array):
  """
  Downsample x by factor using averaging.

  @return: The downsampled array, of the same type as x.
  """

  if len(array.shape) == 3:
    factor = (2,2,1)
  else:
    factor = (2,2)
  
  if np.array_equal(factor[:3], np.array([1,1,1])):
    return array

  output_shape = tuple(int(math.ceil(s / f)) for s, f in zip(array.shape, factor))
  temp = np.zeros(output_shape, float)
  counts = np.zeros(output_shape, np.int)
  for offset in np.ndindex(factor):
      part = array[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
      indexing_expr = tuple(np.s_[:s] for s in part.shape)
      temp[indexing_expr] += part
      counts[indexing_expr] += 1
  return np.cast[array.dtype](temp / counts)

def downsample_with_max_pooling(array):

  factor = (2,2)

  if np.all(np.array(factor, int) == 1):
      return array

  sections = []

  for offset in np.ndindex(factor):
    part = array[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  output = sections[0].copy()

  for section in sections[1:]:
    np.maximum(output, section, output)

  return output

def striding(array): 
  """Downsample x by factor using striding.

  @return: The downsampled array, of the same type as x.
  """
  factor = (2,2)
  if np.all(np.array(factor, int) == 1):
    return array
  return array[tuple(np.s_[::f] for f in factor)]

filename = sys.argv[1]
img = Image.open(filename)
data = np.array(img.getdata(), dtype=np.uint8)

if len(data.shape) == 1:
  n_channels = 1
  reshape = (img.height, img.width)
else:
  n_channels = min(data.shape[1], 3)
  data = data[:, :n_channels]
  reshape = (img.height, img.width, n_channels)

data = data.reshape(reshape).astype(np.uint8)

methods = [
  simplest_countless,
  quick_countless,
  zero_corrected_countless,
  countless,
  downsample_with_averaging,
  downsample_with_max_pooling,
  striding,
  countless_if,
  counting
]

formats = {
  1: 'L',
  3: 'RGB',
  4: 'RGBA'
}

if not os.path.exists('./results'):
  os.mkdir('./results')

N = 100
img_size = img.width * img.height / 1e6
print "N = %d, %dx%d (%.2f MPx) %d chan, %s" % (N, img.width, img.height, img_size, n_channels, filename)
for fn in methods:
  start = time.time()
  
  # tqdm is here to show you what's going on the first time you run it.
  # Feel free to remove it to get slightly more accurate timing results.
  for _ in tqdm(xrange(N), desc=fn.__name__):
    result = fn(data)
  end = time.time()
  print "\r",

  total_time = (end - start)
  mpx = N * float(img.height * img.width) / total_time / 1e6
  mbytes = N * float(img.height * img.width * n_channels) / total_time / 1024.0 / 1024.0
  # Output in tab separated format to enable copy-paste into excel/numbers
  print "%s\t%.3f\t%.3f\t%.2f" % (fn.__name__, mpx, mbytes, total_time)
  img = Image.fromarray(result, formats[n_channels])
  img.save('./results/{}.png'.format(fn.__name__, "PNG"))
