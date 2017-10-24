
/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  Enrico Reimer <reimer@cbs.mpg.de>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <json/value.h>
#include <json/reader.h>
#include <ace/INet/HTTP_Session.h>
#include <ace/INet/HTTP_BasicAuthentication.h>
#include <ace/Log_Msg_Callback.h>
#include <ace/Log_Record.h>

#include <isis/data/io_interface.h>
#include <functional>
#include <iostream>
#include <memory>

#include <isis/data/io_factory.hpp>

namespace isis
{
namespace image_io
{
namespace _internal
{
class AceISISLog : public ACE_Log_Msg_Callback{

	void log(ACE_Log_Record & log_record) override {
	std::unique_ptr<isis::util::Message> message;
		if(!_ENABLE_DEBUG);
		else switch(log_record.priority()) {
			case LM_STARTUP: //Initialize the logger (decimal 64).
			case LM_SHUTDOWN: //Shutdown the logger (decimal 1).
			case LM_TRACE: //Messages indicating function-calling sequence (decimal 2).
			case LM_DEBUG: //Messages that contain information normally of use only when debugging a program (decimal 4).
				message=std::make_unique<isis::util::Message>(isis::util::_internal::Log<Debug>::send(__FILE__,__FUNCTION__,__LINE__,isis::verbose_info));
				break;
			case LM_INFO: //Informational messages (decimal 8).
				message=std::make_unique<isis::util::Message>(isis::util::_internal::Log<Debug>::send(__FILE__,__FUNCTION__,__LINE__,isis::info));
				break;
			}

		if(!_ENABLE_LOG);
		else switch(log_record.priority()) {
			case LM_NOTICE: //Conditions that are not error conditions, but that may require special handling (decimal 16).
				message=std::make_unique<isis::util::Message>(isis::util::_internal::Log<Runtime>::send(__FILE__,__FUNCTION__,__LINE__,isis::notice));
				break;
			case LM_WARNING: //Warning messages (decimal 32).
				message=std::make_unique<isis::util::Message>(isis::util::_internal::Log<Runtime>::send(__FILE__,__FUNCTION__,__LINE__,isis::warning));
				break;
			case LM_ERROR: //Error messages (decimal 128).
			case LM_CRITICAL: //Critical conditions, such as hard device errors (decimal 256).
			case LM_ALERT: //A condition that should be corrected immediately, such as a corrupted system database (decimal 512).
			case LM_EMERGENCY: //A panic condition. This is normally broadcast to all users (decimal 1024).
				message=std::make_unique<isis::util::Message>(isis::util::_internal::Log<Runtime>::send(__FILE__,__FUNCTION__,__LINE__,isis::error));
				break;
			}
		// 		log_record.print(ACE_TEXT(""),ACE_Log_Msg::VERBOSE);
		if(message) {
			std::string msg(log_record.msg_data());
			(*message) << msg.substr(0,msg.find_last_not_of("\n\r"));
		}
	}

public:
	static void enable() {
		ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);
		ACE_LOG_MSG->set_flags(ACE_Log_Msg::MSG_CALLBACK);
		ACE_LOG_MSG->msg_callback(new AceISISLog);
	}
};


class AceSession: public ACE::HTTP::Session {
public:
	boost::variant<Json::Value,std::list<data::Chunk>> get(const ACE_CString &url) {
		ACE::HTTP::Request req(ACE::HTTP::Request::HTTP_GET,url);
		
		LOG(Debug,info) << "Requesting " << url;

		send_request(req);

		ACE::HTTP::Response resp;
		auto &s=receive_response(resp);
		
		auto stat=resp.get_status();
		boost::variant<Json::Value,std::list<data::Chunk>> result;

		if(stat.is_valid()) {
			ACE_CString type;
			resp.get("Content-Type",type);
			if(type=="application/json"){
				result=Json::Value();
				s >> boost::get<Json::Value>(result);
			} else if(type=="application/dicom"){
				try{
					result=data::IOFactory::loadChunks( s.rdbuf(), {"dcm"}, {} );
				} catch(std::runtime_error &){}//ignore any error on single instances
			} else {
				LOG(Runtime,error) << "request " << req.get_URI() << " resulted in unknown result type " << type;
			}
			return result;
		} else {
			LOG(Runtime,error)
				<< "request " << req.get_URI() << " failed with "
				<< resp.get_status().get_status() << "(" << resp.get_status().get_reason() << ")";
		}
		return result;
	}

public:
	AceSession(const ACE_Time_Value timeout = ACE_Time_Value::max_time):ACE::HTTP::Session(timeout) {}

// 	std::list<isis::data::Chunk> getChunk(const std::string &url,isis::data::IOFactory::FileFormatPtr loader) {
// 		std::list<isis::data::Chunk> ret;
// 		auto handler=[&](std::istream &in) {
// 			try
// 			{
// 				ret=loader->load(in.rdbuf(), {"dcm"}, {}, nullptr );
// 			} catch (std::runtime_error &e) {
// 				LOG(Runtime, error) << "Loading image data from " << url << " failed with " << e.what();
// 			}
// 		};
// 
// 		if(request(ACE::HTTP::Request::HTTP_GET,url.c_str(),handler,nullptr,0))
// 			return ret;
// 		else
// 			return std::list<isis::data::Chunk>();
// 	}

};

class visitor : public boost::static_visitor<std::list<data::Chunk>>
{
	AceSession &m_session;
	std::shared_ptr<util::ProgressFeedback> m_feedback;
public:
	visitor(AceSession &session,std::shared_ptr<util::ProgressFeedback> feedback):m_session(session),m_feedback(feedback){}
    std::list<data::Chunk> operator()(const Json::Value &value) const
    {
		if(value.isArray()){ // assume its list of instances
			std::list<data::Chunk> ret;
			if(m_feedback)
				m_feedback->show(value.size(),std::string("Loading ")+std::to_string(value.size())+" instances");
			for(const Json::Value i:value)
				ret.splice(ret.end(), this->operator()(i));
			
			return ret;
		} else {
			std::string request;
			if(value["Type"]=="Patient"){
				request=std::string("/patients/")+value["ID"].asString()+"/instances";
			} else if(value["Type"]=="Study"){
				request=std::string("/studies/")+value["ID"].asString()+"/instances";
			} else if(value["Type"]=="Series"){
				request=std::string("/series/")+value["ID"].asString()+"/instances";
			} else if(value["Type"]=="Instance"){
				request=std::string("/instances/")+value["ID"].asString()+"/file";
			} else {
				LOG(Runtime,error) << "Unknown orthanc object type " << value["Type"].asString();
				return std::list<data::Chunk>();
			}
			auto got=m_session.get(request.c_str());
			return boost::apply_visitor(*this,got);
		}
        return std::list<data::Chunk>();
    }
    
    std::list<data::Chunk> operator()(std::list<data::Chunk> &ch) const
    {
		if(m_feedback)
			m_feedback->progress();
        return ch;
    }
};
}

class ImageFormat_orthanc: public FileFormat
{
	_internal::AceSession session;
public:
	std::list< data::Chunk > load(
	  const boost::filesystem::path &filename,
	  std::list<util::istring> /*formatstack*/,
	  std::list<util::istring> dialects,
	  std::shared_ptr<util::ProgressFeedback> feedback
	)throw( std::runtime_error & ) override{
		
		_internal::AceSession session;
		ACE::HTTP::URL url(filename.c_str());
		
		if(!url.validate())
			throwGenericError("invalid url");
		
		session.set_host(url.get_host());
		session.set_port(url.get_port());
		
		auto result=session.get(url.get_request_uri().c_str());
		
		return boost::apply_visitor(_internal::visitor(session,feedback), result);
	}
	std::string getName() const override{return "orthanc database access";};
	void write(const data::Image & image, const std::string & filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback) override{
		throwGenericError("not implemented");
	}
	ImageFormat_orthanc(){}
protected:
	util::istring suffixes(isis::image_io::FileFormat::io_modes modes) const override{return ".orthanc";}
};

}
}

isis::image_io::FileFormat *factory()
{
	isis::image_io::_internal::AceISISLog::enable();
	return new isis::image_io::ImageFormat_orthanc();
}


#endif // DATABASE_H
