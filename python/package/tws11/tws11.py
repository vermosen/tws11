from abc import ABC
from enum import Enum

from ._tws11 import contract, client

__all__ = ['client', 'currency', 'equity' ]

# base instrument class  
class instrument(ABC):

  class code(Enum):
    # ...
    cash = 6
    # ...

  def __init__(self, contract_):
    self.contract_ = contract_
    pass

  def __str__(self):
    return self.contract_.__str__()

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
  
  def __init__(self):

    c = contract()
    #c.symbol = symbol
    #c.currency = denomination
    #c.exchange = exchange
    #c.type = instrument.code.cash.value
    super().__init__(c)
