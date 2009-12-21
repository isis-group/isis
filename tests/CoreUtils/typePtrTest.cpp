/*
 * typePtrTest.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE TypePtrTest
#include <boost/test/included/unit_test.hpp>
#include "CoreUtils/type.hpp"

namespace isis{namespace test{

struct Deleter{
	static bool deleted;
	void operator()(void *p){
		free(p);
		deleted=true;
	};
};

bool Deleter::deleted=false;
	
BOOST_AUTO_TEST_CASE(typePtr_init_test) {
/*	ENABLE_LOG(util::CoreDebug,util::DefaultMsgPrint,util::verbose_info);
	ENABLE_LOG(util::CoreLog,util::DefaultMsgPrint,util::verbose_info);*/
	
	BOOST_CHECK(not Deleter::deleted);
	{
		util::TypePtr<int> outer;
		// default constructor must create an empty pointer
		BOOST_CHECK_EQUAL(outer.len(),0);
		BOOST_CHECK(not (boost::shared_ptr<int>)outer);
		BOOST_CHECK_EQUAL(((boost::shared_ptr<int>)outer).use_count(),0);
		{
			util::TypePtr<int> inner((int*)calloc(5,sizeof(int)),5,Deleter());

			// for now we have only one pointer referencing the data
			boost::shared_ptr<int> &dummy=inner; //get the smart_pointer inside, because TypePtr does not have/need use_count
			BOOST_CHECK_EQUAL(dummy.use_count(),1);

			outer=inner;//now we have two
			BOOST_CHECK_EQUAL(dummy.use_count(),2);

			//and both reference the same data
			BOOST_CHECK_EQUAL(outer.len(),inner.len());
			BOOST_CHECK_EQUAL(((boost::shared_ptr<int>)outer).get(),((boost::shared_ptr<int>)inner).get());
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int> &dummy=outer;
		BOOST_CHECK_EQUAL(dummy.use_count(),1);
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK(Deleter::deleted);
}

BOOST_AUTO_TEST_CASE(typePtr_splice_test) {

	Deleter::deleted=false;
	{
		std::vector<util::TypePtr<int> > outer;
		{
			util::TypePtr<int> inner((int*)calloc(5,sizeof(int)),5,Deleter());
			
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int> &dummy=inner; //get the smart_pointer inside, because TypePtr does not have/need use_count
			BOOST_CHECK_EQUAL(dummy.use_count(),1);

			//splicing up makes a references for every splice
			outer=inner.splice(2);
			BOOST_CHECK_EQUAL(outer.size(),5/2+1);// 5/2 normal splices plus one halve  splice
			//the shall be outer.size() references from the splices, plus one for the origin
			BOOST_CHECK_EQUAL(dummy.use_count(),outer.size()+1);
		}
		BOOST_CHECK_EQUAL(outer[0].len(),2);// the first slices shall be of the size 2
		BOOST_CHECK_EQUAL(outer.back().len(),1);// the last slice shall be of the size 1 (5%2)

		//we cannot ask for the use_count of the original because its hidden in DelProxy (outer[0].use_count will get the use_count of the splice)
		//but we can check if it was allready deleted (it shouldn't, because the splices are still using that data)
		BOOST_CHECK(not Deleter::deleted);
	}
	//now that all splices are gone the original data shall be deleted
	BOOST_CHECK(Deleter::deleted);
}
}}
