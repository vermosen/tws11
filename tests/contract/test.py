#!/usr/bin/env python

import os, sys
import unittest

class test_contract(unittest.TestCase):

  def __init__(self, *args, **kwargs):
    super(test_contract, self).__init__(*args, **kwargs)
    pass

  def test_create_contract(self):
        
    from tws11 import contract
    
    c = contract()
    self.assertIsNotNone(c)

  def test_print_contract(self):

    from tws11 import contract

    c = contract()
    c.id = 1
    c.symbol   = "EUR"
    c.currency = "USD"
    c.exchange = "IDEALPRO"
    c.type     = "CASH"

    self.assertEqual(c.__str__(), ''.join([
      "contract id: 1\n",
      "symbol:   EUR\n",
      "type:     CASH\n",
      "exchange: IDEALPRO\n",
      "currency: USD\n"
    ]))

if __name__ == '__main__':
  unittest.main()