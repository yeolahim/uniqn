#!/usr/bin/env python3

import sys

maxword = 0
with open(sys.argv[1], 'r') as text:
  for line in text:
    line = ''.join([c if c.isalpha() else ' ' for c in line ]).lower()
    line = line.strip().split()
    if len(line) == 0:
      continue
    for word in line:
      if len(word) > maxword:
        maxword = len(word)
    print('\n'.join(line))
#  print(maxword)
