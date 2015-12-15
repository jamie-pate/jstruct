#!/usr/bin/env python

import sys

sys.path[0:0] = ['.', '..']

import unittest2

suite = unittest2.TestLoader().loadTestsFromNames(['test_parse_jstruct'])

testresult = unittest2.TextTestRunner(verbosity=1).run(suite)
sys.exit(0 if testresult.wasSuccessful() else 1)
