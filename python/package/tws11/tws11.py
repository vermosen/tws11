from abc import ABC
from enum import Enum

from ._tws11 import contract, client

__all__ = ['client', 'contract', 'currency', 'equity', 'option' ]

# base instrument class  
class instrument(ABC):

  # from https://interactivebrokers.github.io/tws-api/classIBApi_1_1Contract.html
  class code(Enum):
    stk     = 1
    opt     = 2
    fut     = 3
    cash    = 6
    # ...

  def __init__(self, contract_):
    self.contract_ = contract_
    pass

  def populate(self, client, timeout = -1, verbose = False):

    try:    
      # this one throws if the contract is misformed
      details = client.details(self.contract_, timeout, verbose)
    
      if len(details) == 0:
        raise AttributeError('contract misspecified. Populate method returned no match !')
      elif len(details) > 1:
        raise AttributeError('contract partially specified. Populate method returned several matches !')
      else:
        self.contract_ = details[0].contract      
      return True
    
    except:
      return False

  def __str__(self):
    return self.contract_.__str__()

  @property
  def contract(self):
    return self.contract_

# derived instrument types
class currency(instrument):
  
  def __init__(self, symbol, denomination, exchange):

    c = contract()
    c.symbol = symbol
    c.currency = denomination
    c.exchange = exchange
    c.type = instrument.code.cash.name.upper()

    super().__init__(c)

class equity(instrument):
  
  def __init__(self, symbol, denomination, exchange):

    c = contract()
    c.symbol = symbol
    c.currency = denomination
    c.exchange = exchange
    c.type = instrument.code.stk.name.upper()

    super().__init__(c)

  # for now we only consider stock options ...
  def get_chain(self, client, exchange='', timeout=-1):
    desc = client.chain(self.contract, exchange, timeout)
    return desc

class option(instrument):
  
  class side(Enum):
    call = 1
    put  = 2

  def __init__(self, underlying, strike, expiry, currency, exchange, side = side.call, multiplier = 100, category=None):

    from datetime import datetime

    tmp = ''
    if type(expiry) == int:
      tmp = str(expiry)
    elif type(expiry) == str:
      tmp = expiry
    elif type(expiry) == datetime:
      # TODO
      # tmp = None  
      pass

    c = contract()
    c.symbol     = underlying
    c.type       = instrument.code.opt.name.upper()
    c.exchange   = exchange
    c.currency   = currency
    c.expiry     = expiry
    c.strike     = strike
    c.side       = "C" if side == side.call else "P"
    c.multiplier = str(multiplier)
    c.category   = underlying if category is None else category
    super().__init__(c)

  # ctor from option description object
  @staticmethod
  def from_description(description, currency, side = side.call):

    retval = option(
      underlying=description.underlying, 
      strike=float(description.strike),
      expiry=str(description.expiry),
      currency=currency,
      exchange=description.exchange,
      side=side,
      multiplier=description.multiplier,
      category=description.category
    )

    return retval
    
    
