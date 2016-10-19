#include <isis/data/io_application.hpp>

using namespace isis;

int main( int argc, char **argv )
{
	util::DefaultMsgPrint::stopBelow(warning);
	data::IOApplication app( "isis data converter", true, true );
	app.addExample( "-in myFile.nii -out myFile.v", "Simple conversion from a nifti file to a vista file." );
	app.addExample(
		"-in directory_full_of_dicom_files -out /tmp/S{sequenceNumber}_{sequenceDescription}_e{echoTime}.nii",
		"Read all dicom-files (*.ima) from the directory given and write the resulting images as nifti files "
		"into /tmp/ generating their names from the sequenceNumber, sequenceDescription and echoTime extracted "
		"from the dicom files." );
	app.init( argc, argv ); // will exit if there is a problem

	app.autowrite( app.images );
	return EXIT_SUCCESS;
}
