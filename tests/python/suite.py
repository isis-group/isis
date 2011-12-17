import unittest
from ndimensionalTest import *
from imageTest import *

#running the tests

unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(TestNDimensional))
unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(TestImage))

