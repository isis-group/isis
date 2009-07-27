/***************************************************************************
 * itkMain.cxx
 *
 * This small program uses the checkerboard filter to constract a
 * checkerboard image composed of the two input images.
 *
 **************************************************************************/

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include "itkCheckerBoardImageFilter.h"
#include <metaCommand.h>

int main(int argc, char** argv) {


  /*******************************************
   * The commandline parser
   ******************************************/

  // At the moment we will use the MetaIO framework from ITK to get the
  // commandline parameters.
  MetaCommand command;

  /* The option definitions. More information can be found at:
   * http://www.itk.org/Wiki/MetaIO/MetaCommand_Documentation
   */

  // '-i / --in' -- the input files
  command.SetOption("Input", "i", true, "input file");
  command.SetOptionLongTag("Input", "in");
  command.AddOptionField("Input", "infile1", MetaCommand::STRING, true);
  command.AddOptionField("Input", "infile2", MetaCommand::STRING, true);


  // '-o / --out' -- the output file
  command.SetOption("Output", "o", true, "output file");
  command.SetOptionLongTag("Output","out");
  command.AddOptionField("Output", "outfile", MetaCommand::STRING, true);

  // parse the commandline and quit if there are any parameter errors
  if(!command.Parse(argc,argv)){
    return 1;
  }

  // get the parameter values
  std::string in_filename1 = command.GetValueAsString("Input","infile1");
  std::string in_filename2 = command.GetValueAsString("Input","infile2");
  std::string out_filename = command.GetValueAsString("Output","outfile");

  /*******************************************
   * some preparations for image conversion
   ******************************************/

  // Choose an internal datatype for the pixel data.
  typedef int PixelType;
  // Set the number of image dimensions.
  const unsigned short dimension = 3;
  // Define the image type according to the pixel type and the image
  // dimension
  typedef itk::Image<PixelType, dimension> ImageType;

  // We now instantiate the image file reader and writer types. They are
  // parametrized over the image type.
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;

  // Now it's time to create some reader and writer objects. After creation
  // with the New() macro the results are assigned to SmartPointer objects.
  ReaderType::Pointer reader1 = ReaderType::New();
  ReaderType::Pointer reader2 = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  itk::CheckerBoardImageFilter<ImageType>::Pointer checker =
	  itk::CheckerBoardImageFilter<ImageType>::New();

  /*******************************************
   * let's start the real action
   ******************************************/

  // Assign the input and output filename to the reader and writer object.
  reader1->SetFileName(in_filename1);
  reader2->SetFileName(in_filename2);
  writer->SetFileName(out_filename);

  checker->SetInput1(reader1->GetOutput());
  checker->SetInput2(reader2->GetOutput());

  // Connect the reader output to the writer input. So we get a minimal image
  // converter.
  writer->SetInput(checker->GetOutput());

  /* Comment: It's very easy to extend this little program to build an arbitrary
   * filter chain with the given input and output images. All additional
   * processing objects can be connected via the 'SetInput'/'GetOutput' paradigm as
   * demonstrated with the file reader and writer objects.
   */

  // START the pipeline
  writer->Update();

}
