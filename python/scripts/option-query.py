#!/usr/bin/env python

from tws11 import *
import pandas as pd
from datetime import date, datetime

def main():
  cl = client(id=2, host='127.0.0.1', port=4002)

  if not cl.connect(timeout=5):
    raise AttributeError('connection failed')

  dt = date(2021, 1, 10)

  ud = equity(symbol='IBM', denomination='USD', exchange='NYSE')
  ud.populate(cl, timeout=5)

  exp = '20210129'

  slice = [option(
    underlying=str(d.category), 
    strike=d.strike, 
    expiry=d.expiry, 
    currency='USD', 
    exchange='BATS', 
    side = option.side.call, 
    multiplier=d.multiplier, 
    category=d.category
  ) for d in ud.get_chain(cl, exchange='CBOE', timeout=20) if str(d.expiry) == exp]

  # get option contract details
  res = cl.details(slice[0].contract, timeout=5, verbose=True)

if __name__ == '__main__':
  main()