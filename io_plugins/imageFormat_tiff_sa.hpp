#include <isis/data/io_interface.h>

namespace isis{
namespace image_io{

class ImageFormat_TiffSa: public FileFormat
{
public:
	std::list< data::Chunk > load(
		data::ByteArray source, 
		std::list<util::istring> /*formatstack*/, 
		const util::istring &dialect, 
		std::shared_ptr<util::ProgressFeedback> feedback
	)throw( std::runtime_error & ) override;
	std::string getName() const override;
	void write(const data::Image & image, const std::string & filename, const util::istring & dialect, std::shared_ptr<util::ProgressFeedback> feedback) override;
protected:
	util::istring suffixes(isis::image_io::FileFormat::io_modes modes) const override;
};

}}
