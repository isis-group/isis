import unittest
from isis import core, data

class TestNDimensional(unittest.TestCase):
	
	def setUp(self):
		self.ndim = data.NDimensional4(core.ivector4(100,200,100,10))
	
	def test_init_with_constructor(self):
		#create a NDimensional object
		dims = core.ivector4(10,10,10,10)
		ndim = data.NDimensional4(dims)
		for i in ndim.getSizeAsVector():
			self.assertTrue( i == 10 )
			
	def test_init_with_init1(self):
		dims = core.ivector4(12,12,12,12)
		ndim = data.NDimensional4()
		ndim.init(dims)
		for i in ndim.getSizeAsVector():
			self.assertTrue( i == 12 )
	
	def test_init_with_init2(self):
		ndim = data.NDimensional4()
		ndim.init(13,13,13,13)
		for i in ndim.getSizeAsVector():
			self.assertTrue( i == 13 )
			
	def test_get_linear_index1(self):
		self.assertTrue( self.ndim.getLinearIndex(core.ivector4(0,0,0,0)) == 0 )
		self.assertTrue( self.ndim.getLinearIndex(core.ivector4(99,199,99,9)) == (100 * 200 * 100 * 10 - 1) )
		
	def test_get_linear_index2(self):
		self.assertTrue( self.ndim.getLinearIndex(0,0,0,0) == 0 )
		self.assertTrue( self.ndim.getLinearIndex(99,199,99,9) == (100 * 200 * 100 * 10 - 1) )
	
	def test_get_coords_from_linear_index(self):
		for i in self.ndim.getCoordsFromLinIndex(0):
			self.assertTrue( i == 0 )
		self.assertTrue( self.ndim.getCoordsFromLinIndex((100 * 200 * 100 * 10 - 1))[0] == 99)
		self.assertTrue( self.ndim.getCoordsFromLinIndex((100 * 200 * 100 * 10 - 1))[1] == 199)
		self.assertTrue( self.ndim.getCoordsFromLinIndex((100 * 200 * 100 * 10 - 1))[2] == 99)
		self.assertTrue( self.ndim.getCoordsFromLinIndex((100 * 200 * 100 * 10 - 1))[3] == 9)
		
	def test_range_check1(self):
		self.assertTrue( self.ndim.isInRange(core.ivector4(0,0,0,0) ) )
		self.assertTrue( self.ndim.isInRange(core.ivector4(99,199,99,9) ) )
		self.assertTrue( not self.ndim.isInRange(core.ivector4(100,199,99,9) ) )
		
	def test_range_check2(self):
		self.assertTrue( self.ndim.isInRange(0,0,0,0) ) 
		self.assertTrue( self.ndim.isInRange(99,199,99,9) )
		self.assertTrue( not self.ndim.isInRange(100,199,99,9) )
		
	def test_get_volume(self):
		self.assertTrue( self.ndim.getVolume() == 100 * 200 * 100 * 10 )
		
	def test_get_dim_size(self):
		self.assertTrue( self.ndim.getDimSize(data.dimensions.ROW_DIM) == 100 )
		self.assertTrue( self.ndim.getDimSize(data.dimensions.COLUMN_DIM) == 200 )
		self.assertTrue( self.ndim.getDimSize(data.dimensions.SLICE_DIM) == 100 )
		self.assertTrue( self.ndim.getDimSize(data.dimensions.TIME_DIM) == 10 )
		self.assertTrue( self.ndim.getDimSize(0) == 100 )
		self.assertTrue( self.ndim.getDimSize(1) == 200 )
		self.assertTrue( self.ndim.getDimSize(2) == 100 )
		self.assertTrue( self.ndim.getDimSize(3) == 10 )
	
	def test_get_size_as_string(self):
		self.assertTrue( self.ndim.getSizeAsString() == "100x200x100x10")
		self.assertTrue( self.ndim.getSizeAsString(":") == "100:200:100:10")
	
	def test_get_size_as_vector(self):
		self.assertTrue( self.ndim.getSizeAsVector()[0] == 100)
		self.assertTrue( self.ndim.getSizeAsVector()[1] == 200)
		self.assertTrue( self.ndim.getSizeAsVector()[2] == 100)
		self.assertTrue( self.ndim.getSizeAsVector()[3] == 10)
		
	def test_get_relevant_dims(self):
		self.assertTrue( self.ndim.getRelevantDims() == 4 )
		self.assertTrue( data.NDimensional4(core.ivector4(1,64,1,1)).getRelevantDims() == 2)
		
	def test_get_fov(self):
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(0,0,0,0) )[0] == 50 )
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(0,0,0,0) )[1] == 100 )
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(0,0,0,0) )[2] == 50 )
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(0,0,0,0) )[3] == 5 )
		
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(1,1,1,1) )[0] == 149 )
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(1,1,1,1) )[1] == 299 )
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(1,1,1,1) )[2] == 149 )
		self.assertTrue( self.ndim.getFoV( core.fvector4(0.5,0.5,0.5,0.5), core.fvector4(1,1,1,1) )[3] == 14 )
		
	
		
if __name__ == "__main__":
	unittest.main()