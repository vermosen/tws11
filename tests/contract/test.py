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
      "symbol:     EUR\n",
      "type:       CASH\n",
      "exchange:   IDEALPRO\n",
      "currency:   USD\n"
      "multiplier: \n"
      "side:       \n"
      "strike:     0\n"
      "expiry:     \n"
      "category:   \n"
    ]))

  def test_build_currency(self):

    from tws11 import currency

    c = currency(symbol = "EUR", denomination = "USD", exchange = "IDEALPRO")

    self.assertEqual(c.contract.__str__(), ''.join([
        "contract id: 0\n",
        "symbol:     EUR\n",
        "type:       CASH\n",
        "exchange:   IDEALPRO\n",
        "currency:   USD\n"
        "multiplier: \n"
        "side:       \n"
        "strike:     0\n"
        "expiry:     \n"
        "category:   \n"
    ]))

  def test_build_equity(self):

    from tws11 import equity

    c = equity(symbol = "AAPL", denomination = "USD", exchange = "BATS")

    self.assertEqual(c.contract.__str__(), ''.join([
        "contract id: 0\n",
        "symbol:     AAPL\n",
        "type:       STK\n",
        "exchange:   BATS\n",
        "currency:   USD\n"
        "multiplier: \n"
        "side:       \n"
        "strike:     0\n"
        "expiry:     \n"
        "category:   \n"
    ]))

  def test_build_option(self):

    from tws11 import option

    c = option(underlying='265598', strike=100.50, expiry='20210114', currency='USD', exchange='BATS', side = option.side.call, multiplier = 100, category='AAPL')

    self.assertEqual(c.contract.__str__(), ''.join([
        "contract id: 0\n",
        "symbol:     265598\n",
        "type:       OPT\n",
        "exchange:   BATS\n",
        "currency:   USD\n"
        "multiplier: 100\n"
        "side:       C\n"
        "strike:     100.5\n"
        "expiry:     20210114\n"
        "category:   AAPL\n"
    ]))

if __name__ == '__main__':
  unittest.main()