#!/usr/bin/env python

from tws11 import *
import pandas as pd
from datetime import date, datetime

def main():
  cl = client(id=1, host='127.0.0.1', port=4002)

  if not cl.connect(timeout=5):
    raise AttributeError('connection failed')

  dt = date(2021, 1, 10)

  c = equity(symbol='IBM', denomination='USD', exchange='NYSE')
  c.populate(cl, timeout=5)
  res = cl.request_bulk([c.contract], 'MIDPOINT', '30 mins', '1 D', dt, timeout=5)
  df = pd.DataFrame.from_dict(res)

if __name__ == '__main__':
  main()