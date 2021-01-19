from abc import ABC

from ._tws11 import contract

__all__ = ['contract']

# instrument inheritance
class instrument(object):
  # cast into a contract
  pass

class currency(instrument):
  pass

class equity(instrument):
  pass
