#include <isis/core/io_interface.h>

namespace isis{
namespace image_io{

class ImageFormat_TiffSa: public FileFormat
{
public:
	std::list< data::Chunk > load(
		data::ByteArray source, 
		std::list<util::istring> /*formatstack*/, 
		std::list<util::istring> dialects, 
		std::shared_ptr<util::ProgressFeedback> feedback
	) override;
	std::string getName() const override;
	std::list<util::istring> dialects() const override;
	void write(const data::Image & image, const std::string & filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback) override;
protected:
	util::istring suffixes(isis::image_io::FileFormat::io_modes modes) const override;
};

}}
