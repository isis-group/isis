/*
 * main.cxx
 *
 *  Created on: May 14, 2009
 *      Author: tuerke
 */

//programm related stuff
//#include "isisTimeStepExtractionFilter.h"
#include "isisSTDEVMaskFilter.h"
#include "isisTimeStepExtractionFilter.h"
#include "isisGradientMagnitudeSegmentationFilter.h"


#include <iostream>


//regestration related headers
#include "itkImageRegistrationMethod.h"
#include "itkVersorRigid3DTransform.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkAffineTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkMeanSquaresImageToImageMetric.h"


//deformable registration stuff
#include "itkBSplineDeformableTransform.h"
#include "itkLBFGSBOptimizer.h"




#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

//registration related files

#include "itkCommand.h"
#include "metaCommand.h"


#include "itkImageLinearConstIteratorWithIndex.h"


class CommandIterationUpdate : public itk::Command
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate() {};
public:
  typedef itk::RegularStepGradientDescentOptimizer     OptimizerType;
  typedef   const OptimizerType   *    OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event);
  }

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer =
                      dynamic_cast< OptimizerPointer >( object );
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
      std::cout << optimizer->GetCurrentIteration() << "   ";
      std::cout << optimizer->GetValue() << "   ";
      std::cout << optimizer->GetCurrentPosition() << std::endl;
/*
      // Print the angle for the trace plot
      vnl_matrix<double> p(2, 2);
      p[0][0] = (double) optimizer->GetCurrentPosition()[0];
      p[0][1] = (double) optimizer->GetCurrentPosition()[1];
      p[1][0] = (double) optimizer->GetCurrentPosition()[2];
      p[1][1] = (double) optimizer->GetCurrentPosition()[3];
      vnl_svd<double> svd(p);
      vnl_matrix<double> r(2, 2);
      r = svd.U() * vnl_transpose(svd.V());
      double angle = asin(r[1][0]);
      std::cout << " AffineAngle: " << angle * 45.0 / atan(1.0) << std::endl;
      */
    }
};

int main( int argc, char** argv)
{
	const std::string progName = "isTimeSeries";


	const unsigned int fmriDimension = 4;
	const unsigned int MNIDimension = 3;
	const unsigned int fmriDimensionOut = 3;

	typedef short MNIPixelType;

	typedef itk::Image< float, 3 > FloatImageType;
	typedef itk::Image< short, MNIDimension > MNIImageType;
	typedef itk::Image< short, MNIDimension > OutputImageType;
	typedef itk::Image< MNIPixelType, fmriDimension > FMRIImageType;



	typedef itk::Image< short, 3 > FixedImageType;
	typedef itk::Image< short, 3 > MovingImageType;
	typedef itk::Image< short, 3 > InternalImageType;

	//filter for extracting a timestep from the fMRT-dataset
	typedef isis::TimeStepExtractionFilter< FMRIImageType, OutputImageType >
	TimeStepExtractionFilterType;

	TimeStepExtractionFilterType::Pointer extractionFilter =
			TimeStepExtractionFilterType::New();

	//reader and writer
	typedef itk::ImageFileReader< FMRIImageType > FMRIReaderType;
	typedef itk::ImageFileReader< MNIImageType > FixedReaderType;
	typedef itk::ImageFileReader< MovingImageType > MovingReaderType;
	typedef itk::ImageFileWriter< OutputImageType > WriterType;


	FMRIReaderType::Pointer fmriReader = FMRIReaderType::New();
	FixedReaderType::Pointer fixedReader = FixedReaderType::New();
	MovingReaderType::Pointer movingReader = MovingReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	InternalImageType::Pointer internalImage = InternalImageType::New();
	MovingImageType::Pointer movingImage = MovingImageType::New();
	FixedImageType::Pointer fixedImage = FixedImageType::New();



	CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
/************************************************************************************
 * 		registration stuff
 ************************************************************************************/

	typedef itk::ImageRegistrationMethod < FixedImageType, MovingImageType >
	ImageRegistrationMethodType;

	typedef itk::VersorRigid3DTransform< double > RigidTransformType;

	typedef itk::AffineTransform< double, 3 > AffineTransformType;

	typedef itk::BSplineDeformableTransform< double, 3, 3 > DeformableTransformType;

	typedef itk::RegularStepGradientDescentOptimizer       OptimizerType;

	typedef itk::MattesMutualInformationImageToImageMetric< FixedImageType, MovingImageType >
	MetricType;

	//typedef itk::MeanSquaresImageToImageMetric< FixedImageType, MovingImageType >
	//MetricType;

	typedef itk::LinearInterpolateImageFunction< MovingImageType, double >
	InterpolatorType;

	typedef ImageRegistrationMethodType::ParametersType ParametersType;

	typedef itk::ResampleImageFilter< MovingImageType, FixedImageType >
	ResampleFilterType;

	typedef itk::CastImageFilter< FloatImageType, OutputImageType >
	CastFilterType;

	typedef itk::CastImageFilter< FixedImageType, MovingImageType >
	RegistrationCastFilterType;


	typedef itk::CenteredTransformInitializer< RigidTransformType, FixedImageType, MovingImageType >
	TransformInitializerType;

	/**************************************************
   * user interface
   ****************************************************/

	MetaCommand command;
	command.DisableDeprecatedWarnings();


	//-ref -- the fixed image (MNI atlas)
	command.SetOption("mni", "mni", true, "The MNI atlas.");
	command.AddOptionField("mni", "refImageName", MetaCommand::STRING, true);

	// -in -- the moving image file
	command.SetOption("fmri", "fmri", true, "The fmri image.");
	command.AddOptionField("fmri", "movingImageName", MetaCommand::STRING, true);
/*
	//-timestep -- the desired timestep picked from the fmri image
	command.SetOption("timestep", "timestep", true, "Timestep of the fmri image.");
	command.AddOptionField("timestep", "ntimestep", MetaCommand::INT, true);

	// -out -- the output file
	command.SetOption("output", "out", true, "The result of the registration process");
	command.AddOptionField("output", "resultImageName", MetaCommand::STRING, true);

	// -iter -- number of iterations
	command.SetOption("iter", "iter", false, "Number of optimizer iterations.");
	command.AddOptionField("iter", "niterations", MetaCommand::INT, true, "200");

	// bins - bumber of bins to compute the entropy
	command.SetOption("bins", "bins", false, "Number of bins for calculating the entropy value");
	command.AddOptionField("bins", "nbins", MetaCommand::INT, true, "30");

	// gridSizeOnImage
	command.SetOption("gridSizeOnImage", "gridSizeOnImage", false, "gridSize used by the BSplineDeformableTransfom" );
	command.AddOptionField("gridSizeOnImage", "gridSize", MetaCommand::INT, true, "5");

	// gridBoderSize
	command.SetOption("gridBorderSize", "gridBorderSize", false, "gridBorderSize used by the BSplineDeformableTransforn");
	command.AddOptionField("gridBorderSize", "borderSize", MetaCommand::INT, true, "3");

	command.SetOption("affine", "affine",false, "Using the affine transform");
	command.SetOption("deformable", "deformable",false, "Using the deformable transform");

*/
	// parse the commandline and quit if there are any parameter errors
	if (!command.Parse(argc, argv)) {
	   std::cout << progName << ": ERROR parsing arguments." << std::endl
	   << "Usage: " << progName
	   << " -in <moving image> -ref <fixed image> [optional Options]"
	   << " -out <output image>" << std::endl;
	   return EXIT_FAILURE;
	 }

	typedef isis::GradientMagnitudeSegmentationFilter< MNIImageType, FloatImageType >
							GradientMagnitudeFilterType;

	GradientMagnitudeFilterType::Pointer gradientFilter1 = GradientMagnitudeFilterType::New();
	GradientMagnitudeFilterType::Pointer gradientFilter2 = GradientMagnitudeFilterType::New();

	CastFilterType::Pointer caster = CastFilterType::New();
	RegistrationCastFilterType::Pointer registrationCaster =
			RegistrationCastFilterType::New();


	typedef isis::STDEVMaskFilter< FMRIImageType, FloatImageType > STDEVFilterType;

	STDEVFilterType::Pointer stdevFilter = STDEVFilterType::New();



/*
	fmriReader->SetFileName( command.GetValueAsString("fmri", "movingImageName") );
	fmriReader->Update();

	fixedReader->SetFileName( command.GetValueAsString("mni", "refImageName") );
	fixedReader->Update();


	extractionFilter->SetInput( fmriReader->GetOutput() );
	extractionFilter->SetRequestedTimeStep( 10 );
	extractionFilter->Update();

	gradientFilter1->SetInput( extractionFilter->GetOutput() );
	gradientFilter1->SetSigma( 0.1 );
	gradientFilter1->SetSmoothingTimeStep( 0.05 );
	gradientFilter1->Update();

	caster->SetInput( gradientFilter1->GetOutput() );
	writer->SetInput( caster->GetOutput() );
	writer->SetFileName( "fmriGradientMagnitude.nii");
	writer->Update();

	gradientFilter2->SetInput( fixedReader->GetOutput() );
	gradientFilter2->SetSigma( 0.1 );
	gradientFilter2->SetSmoothingTimeStep( 0.05 );
	gradientFilter2->Update();

	caster->SetInput( gradientFilter2->GetOutput() );
	writer->SetInput( caster->GetOutput() );
	writer->SetFileName( "MNIGradientMagnitude.nii");
	writer->Update();

*/

	 MetricType::Pointer         metric        = MetricType::New();
	 OptimizerType::Pointer      optimizer     = OptimizerType::New();
	 InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
	 ImageRegistrationMethodType::Pointer   registration  = ImageRegistrationMethodType::New();

	 registration->SetMetric(        metric        );
	 registration->SetOptimizer(     optimizer     );
	 registration->SetInterpolator(  interpolator  );
	 AffineTransformType::Pointer  transform = AffineTransformType::New();
	 RigidTransformType::Pointer rigidTransform = RigidTransformType::New();

	 metric->SetNumberOfHistogramBins( 30 );
	 metric->SetNumberOfSpatialSamples( 500 );

	 registration->SetTransform( rigidTransform );
	 movingReader->SetFileName( command.GetValueAsString("fmri", "movingImageName") );
	 movingReader->Update();

	 fixedReader->SetFileName( command.GetValueAsString("mni", "refImageName") );
	 fixedReader->Update();

	 registration->SetFixedImage(    fixedReader->GetOutput()    );
	 registration->SetMovingImage(   movingReader->GetOutput()   );
	 fixedReader->Update();
	 registration->SetFixedImageRegion(
	      fixedReader->GetOutput()->GetBufferedRegion() );
	 typedef itk::CenteredTransformInitializer<
	                                     RigidTransformType,
	                                     FixedImageType,
	                                     MovingImageType >  TransformInitializerType;
	 TransformInitializerType::Pointer initializer = TransformInitializerType::New();
	 initializer->SetTransform(   rigidTransform );
	 initializer->SetFixedImage(  fixedReader->GetOutput() );
	 initializer->SetMovingImage( movingReader->GetOutput() );
	 initializer->GeometryOn();
	 initializer->InitializeTransform();
	 registration->SetInitialTransformParameters(
	                                  rigidTransform->GetParameters() );
	 double translationScale = 1.0 / 1000.0;
	 typedef OptimizerType::ScalesType       OptimizerScalesType;
	 OptimizerScalesType optimizerScales( rigidTransform->GetNumberOfParameters() );
/*
	 for( unsigned int i = 0; i < 6; i++)
	 {
		 optimizerScales[i] =  1.0;
	 }
	 for( unsigned int i = 6; i < 12; i++)
	 {
	 	 optimizerScales[i] =  translationScale;
	 }
*/
	 optimizerScales[0] = 1.0;
	 optimizerScales[1] = translationScale;
	 optimizerScales[2] = translationScale;
	 optimizerScales[3] = translationScale;
	 optimizerScales[4] = translationScale;
	 optimizerScales[5] = translationScale;

	 optimizer->SetScales( optimizerScales );
	 double steplength = 0.1;

	 optimizer->SetMaximumStepLength( steplength );
	 optimizer->SetMinimumStepLength( 0.0001 );
	 optimizer->SetNumberOfIterations( 200 );
	 optimizer->MinimizeOn();
	 optimizer->AddObserver( itk::IterationEvent(), observer );

	 try
	   {
	   registration->StartRegistration();
	   }
	  catch( itk::ExceptionObject & err )
	   {
	   std::cerr << "ExceptionObject caught !" << std::endl;
	   std::cerr << err << std::endl;
	   return EXIT_FAILURE;
	   }


	  typedef itk::ResampleImageFilter<
	                              MovingImageType,
	                              FixedImageType >    ResampleFilterType;
	  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
	  resampler->SetTransform( registration->GetOutput()->Get() );
	  resampler->SetInput( movingReader->GetOutput() );

	  fixedImage = fixedReader->GetOutput();

	  resampler->SetSize(    fixedImage->GetLargestPossibleRegion().GetSize() );
	  resampler->SetOutputOrigin(  fixedImage->GetOrigin() );
	  resampler->SetOutputSpacing( fixedImage->GetSpacing() );
	  resampler->SetOutputDirection( fixedImage->GetDirection() );
	  resampler->SetDefaultPixelValue( 0 );
	  registrationCaster->SetInput( resampler->GetOutput() );
	  writer->SetInput( registrationCaster->GetOutput()   );
	  writer->SetFileName( "Registered.nii" );
	  writer->Update();





	return 0;

}

