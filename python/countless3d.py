from builtins import range
from PIL import Image
import numpy as np
import io
import time
import math
import random
import sys
from collections import defaultdict
from copy import deepcopy
from itertools import combinations
from functools import reduce
from tqdm import tqdm

def countless5(a,b,c,d,e):
  """First stage of generalizing from countless2d. 

  You have five slots: A, B, C, D, E

  You can decide if something is the winner by first checking for 
  matches of three, then matches of two, then picking just one if 
  the other two tries fail. In countless2d, you just check for matches
  of two and then pick one of them otherwise.

  Unfortunately, you need to check ABC, ABD, ABE, BCD, BDE, & CDE.
  Then you need to check AB, AC, AD, BC, BD
  We skip checking E because if none of these match, we pick E. We can
  skip checking AE, BE, CE, DE since if any of those match, E is our boy
  so it's redundant.

  So countless grows cominatorially in complexity.
  """
  sections = [ a,b,c,d,e ]

  p2 = lambda q,r: q * (q == r) # q if p == q else 0
  p3 = lambda q,r,s: q * ( (q == r) & (r == s) ) # q if q == r == s else 0

  lor = lambda x,y: x + (x == 0) * y

  results3 = ( p3(x,y,z) for x,y,z in combinations(sections, 3) )
  results3 = reduce(lor, results3)

  results2 = ( p2(x,y) for x,y in combinations(sections[:-1], 2) )
  results2 = reduce(lor, results2)

  return reduce(lor, (results3, results2, e))

def countless8(a,b,c,d,e,f,g,h):
  """Extend countless5 to countless8. Same deal, except we also
    need to check for matches of length 4."""
  sections = [ a, b, c, d, e, f, g, h ]
  
  p2 = lambda q,r: q * (q == r)
  p3 = lambda q,r,s: q * ( (q == r) & (r == s) )
  p4 = lambda p,q,r,s: p * ( (p == q) & (q == r) & (r == s) )

  lor = lambda x,y: x + (x == 0) * y

  results4 = ( p4(x,y,z,w) for x,y,z,w in combinations(sections, 4) )
  results4 = reduce(lor, results4)

  results3 = ( p3(x,y,z) for x,y,z in combinations(sections, 3) )
  results3 = reduce(lor, results3)

  # We can always use our shortcut of omitting the last element
  # for N choose 2 
  results2 = ( p2(x,y) for x,y in combinations(sections[:-1], 2) )
  results2 = reduce(lor, results2)

  return reduce(lor, [ results4, results3, results2, h ])

def dynamic_countless3d(data):
  """countless8 + dynamic programming. ~2x faster"""
  sections = []

  # shift zeros up one so they don't interfere with bitwise operators
  # we'll shift down at the end
  data += 1 
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  factor = (2,2,2)
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)
  
  pick = lambda a,b: a * (a == b)
  lor = lambda x,y: x + (x == 0) * y

  subproblems2 = {}

  results2 = None
  for x,y in combinations(range(7), 2):
    res = pick(sections[x], sections[y])
    subproblems2[(x,y)] = res
    if results2 is not None:
      results2 += (results2 == 0) * res
    else:
      results2 = res

  subproblems3 = {}

  results3 = None
  for x,y,z in combinations(range(8), 3):
    res = pick(subproblems2[(x,y)], sections[z])

    if z != 7:
      subproblems3[(x,y,z)] = res

    if results3 is not None:
      results3 += (results3 == 0) * res
    else:
      results3 = res

  subproblems2 = None # free memory

  results4 = ( pick(subproblems3[(x,y,z)], sections[w]) for x,y,z,w in combinations(range(8), 4) )
  results4 = reduce(lor, results4) 
  subproblems3 = None # free memory

  final_result = reduce(lor, (results4, results3, results2, sections[-1])) - 1
  data -= 1
  return final_result

def countless3d(data):
  """Now write countless8 in such a way that it could be used
  to process an image."""
  sections = []

  # shift zeros up one so they don't interfere with bitwise operators
  # we'll shift down at the end
  data = data + 1 
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  factor = (2,2,2)
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  p2 = lambda q,r: q * (q == r)
  p3 = lambda q,r,s: q * ( (q == r) & (r == s) )
  p4 = lambda p,q,r,s: p * ( (p == q) & (q == r) & (r == s) )

  lor = lambda x,y: x + (x == 0) * y

  results4 = ( p4(x,y,z,w) for x,y,z,w in combinations(sections, 4)  )
  results4 = reduce(lor, results4)

  results3 = ( p3(x,y,z) for x,y,z in combinations(sections, 3)  )
  results3 = reduce(lor, results3)

  results2 = ( p2(x,y) for x,y in combinations(sections[:-1], 2)  )
  results2 = reduce(lor, results2)

  return reduce(lor, (results4, results3, results2, sections[-1])) - 1

def countless_generalized(data, factor):
  assert len(data.shape) == len(factor)

  sections = []

  mode_of = reduce(lambda x,y: x * y, factor)
  majority = int(math.ceil(float(mode_of) / 2))

  data = data + 1
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  def pick(elements):
    eq = ( elements[i] == elements[i+1] for i in range(len(elements) - 1) )
    anded = reduce(lambda p,q: p & q, eq)
    return elements[0] * anded

  def logical_or(x,y):
    return x + (x == 0) * y

  result = ( pick(combo) for combo in combinations(sections, majority) )
  result = reduce(logical_or, result)
  for i in range(majority - 1, 3-1, -1): # 3-1 b/c of exclusive bounds
    partial_result = ( pick(combo) for combo in combinations(sections, i) )
    partial_result = reduce(logical_or, partial_result)
    result = logical_or(result, partial_result)

  partial_result = ( pick(combo) for combo in combinations(sections[:-1], 2) )
  partial_result = reduce(logical_or, partial_result)
  result = logical_or(result, partial_result)

  return logical_or(result, sections[-1]) - 1

def dynamic_countless_generalized(data, factor):
  assert len(data.shape) == len(factor)

  sections = []

  mode_of = reduce(lambda x,y: x * y, factor)
  majority = int(math.ceil(float(mode_of) / 2))

  data = data + 1
  
  # This loop splits the 2D array apart into four arrays that are
  # all the result of striding by 2 and offset by (0,0), (0,1), (1,0), 
  # and (1,1) representing the A, B, C, and D positions from Figure 1.
  for offset in np.ndindex(factor):
    part = data[tuple(np.s_[o::f] for o, f in zip(offset, factor))]
    sections.append(part)

  pick = lambda a,b: a * (a == b)
  lor = lambda x,y: x + (x == 0) * y

  subproblems = [ {}, {} ]
  results2 = None
  for x,y in combinations(range(len(sections) - 1), 2):
    res = pick(sections[x], sections[y])
    subproblems[0][(x,y)] = res
    if results2 is not None:
      results2 = lor(results2, res)
    else:
      results2 = res

  results = [ results2 ]
  for r in range(3, majority+1):
    r_results = None
    for combo in combinations(range(len(sections)), r):
      res = pick(subproblems[0][combo[:-1]], sections[combo[-1]])
      subproblems[1][combo] = res
      if r_results is not None:
        r_results = lor(r_results, res)
      else:
        r_results = res
    results.append(r_results)
    subproblems[0] = subproblems[1]
    subproblems[1] = {}
    
  results.reverse()
  return lor(reduce(lor, results), sections[-1]) - 1

def test(fn):
  alldifferent = [
    [
      [1,2],
      [3,4],
    ],
    [
      [5,6],
      [7,8]
    ]
  ]
  allsame = [
    [
      [1,1],
      [1,1],
    ],
    [
      [1,1],
      [1,1]
    ]
  ]

  assert fn(np.array(alldifferent)) == [[[8]]]
  assert fn(np.array(allsame)) == [[[1]]]

  twosame = deepcopy(alldifferent)
  twosame[1][1][0] = 2

  assert fn(np.array(twosame)) == [[[2]]]

  threemixed = [
    [
      [3,3],
      [1,2],
    ],
    [
      [2,4],
      [4,3]
    ]
  ]
  assert fn(np.array(threemixed)) == [[[3]]]

  foursame = [
    [
      [4,4],
      [1,2],
    ],
    [
      [2,4],
      [4,3]
    ]
  ]

  assert fn(np.array(foursame)) == [[[4]]]

  fivesame = [
    [
      [5,4],
      [5,5],
    ],
    [
      [2,4],
      [5,5]
    ]
  ]

  assert fn(np.array(fivesame)) == [[[5]]]

test(countless3d)
test(dynamic_countless3d)
test(lambda x: countless_generalized(x, (2,2,2)))
test(lambda x: dynamic_countless_generalized(x, (2,2,2)))

block = np.zeros(shape=(512, 512, 512), dtype=np.uint8) + 1

start = time.clock()
ct = 1
for _ in tqdm(range(ct)):
  dynamic_countless3d(block)
  # countless3d(block)
  # countless_generalized(block, (2,2,2))
  # dynamic_countless_generalized(block, (2,2,2))
end = time.clock()

sec = end - start
mpx = ct * (block.shape[0] * block.shape[1] * block.shape[2]) / sec / 1e6
print("{}\t{}".format(sec, mpx))

# block = np.zeros(shape=(1536,1536), dtype=np.uint8) + 1

# print "i\tj\ti*j\tsec\tMpx/sec"
# for i in range(2,5):
#   for j in range(2,5,1):
#     # print "{}x{} ({})".format(i,j,i*j)
#     start = time.clock()
#     ct = 5
#     for _ in range(ct):
#       countless_generalized(block, (i,j))
#     end = time.clock()

#     sec = end - start
#     mpx = ct * (1536.0 * 1536.0) / sec / 1e6
#     print "{}\t{}\t{}\t{}\t{}".format(i,j,i*j,sec, mpx)





