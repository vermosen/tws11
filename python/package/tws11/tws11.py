from abc import ABC
from enum import Enum

from ._tws11 import contract, client

__all__ = ['client', 'currency', 'equity' ]

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

  def __str__(self):
    return self.contract_.__str__()

  @property
  def contract(self):
    return contract_

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
