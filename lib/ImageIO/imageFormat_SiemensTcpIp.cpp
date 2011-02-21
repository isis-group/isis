/*
 *  imageFormat_SiemensTcpIp.cpp
 *  BARTApplication
 *
 *  Created by Lydia Hellrung on 12/10/10.
 *  Copyright 2010 MPI Cognitive and Human Brain Sciences Leipzig. All rights reserved.
 *
 */


#include <DataStorage/io_interface.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace isis
{
	namespace image_io
	{
		
		
		class ImageFormat_SiemensTcpIp: public FileFormat
		{
			
		protected:
			std::string suffixes()const {
				return std::string( ".tcpip" );
			}
		public:
			std::string getName()const {
				return "SiemensTcpIp";
			}
			
			int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
				
                
                /*
                 *TIMESTAMPS
                 */
                printf("IMAGE: %d\n", image_counter);
                
                
                
				firstHeaderArrived = false;
				unsigned long data_received = 0L;
                std::string header_start = "<data_block_header>";
                std::string header_end =  "</data_block_header>";
                std::string session_terminus = "<series_finished>";
                unsigned int byteSize = 0;
                char* dataBuffer;
                std::string header;
             	
				for(;;){
				memset(buffer, 0, sizeof(buffer));
				unsigned int length = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&receiver_address, &receiver_address_length);
				if(length == 0) {
					exit(0);
				}
                    
                     //TODO: If this was the last volume return something empty, something like:
                    // std::string lastVolume = <lastVolume>
                    // if (memcmp(buffer, lastVolume.c_str(), sizeof(lastVolume) -1 ) == 0 ){
                    //  return 0;}
                    if (memcmp(buffer, session_terminus.c_str(), sizeof(session_terminus) - 1) == 0){
                        return 0;
                    }
                        
                    
                    
				
				if(memcmp(buffer, header_start.c_str(), sizeof(header_start) - 1) == 0) {
					firstHeaderArrived = true;
					//printf("[receiver2] received block with header\n");
					data_received = 0L;
					unsigned int header_length = 0;
					while(memcmp(header_end.c_str(), (buffer + header_length), sizeof(header_end)) != 0) {
						header_length++;
					}
					header_length += sizeof(header_end);
                    header = std::string(buffer, header_length);
                    
                    byteSize = atoi(getStringFromHeader("data_size_in_bytes", header).c_str());
					
					//printf("byteSize: '%i'\n", byteSize);
					dataBuffer = (char*)malloc(byteSize);
					
					// copy the first bit of data
					memcpy(dataBuffer, (buffer + header_length), sizeof(buffer) - header_length);
					
					data_received += sizeof(buffer) - header_length;
                    time(&startTime);
					
				} else if (true == firstHeaderArrived) {
					
                   if(byteSize - data_received > 32768) {
						memcpy(dataBuffer + data_received, buffer, sizeof(buffer));
						data_received += sizeof(buffer);
					} else {
						// image complete
                        
                        size_t width = atoi(getStringFromHeader("width", header).c_str());
                        bool moco = getStringFromHeader("motion_corrected", header).compare("yes") == 0 ? true : false;
                        size_t height = atoi(getStringFromHeader("height", header).c_str());
                        bool mosaic = getStringFromHeader("mosaic", header).compare("yes") == 0 ? true : false;
                        size_t iim = 1;
                        //printf("Header \n %s \n", header.c_str());
                        if ( true == mosaic){
                            iim = atoi(getStringFromHeader("images_in_mosaic", header).c_str());
                        }
                        
                        std::string data_type = getStringFromHeader("data_type", header);
                        
                        // Mosaics are always quadratic, so don't bother 'bout only looking for the rows
                        size_t slices_in_row = static_cast<size_t> (ceil(sqrt(static_cast<double_t> (iim) )));
                        size_t width_slice = width / slices_in_row;
                        size_t height_slice = height / slices_in_row;
                        
                        size_t acq_nr = atoi(getStringFromHeader("acquisition_number", header).c_str());
                        std::string seq_descr = getStringFromHeader("sequence_description", header);
                        size_t seq_number = atoi(getStringFromHeader("meas_uid", header).c_str());
                        size_t acq_time = atoi(getStringFromHeader("acquisition_time", header).c_str());
                        uint16_t rep_time = atol(getStringFromHeader("repetition_time", header).c_str());
                        util::fvector4 read_vec = getVectorFromString(getStringFromHeader("read_vector", header));
                        util::fvector4 phase_vec = getVectorFromString(getStringFromHeader("phase_vector", header));
                        util::fvector4 slice_norm_vec = getVectorFromString(getStringFromHeader("slice_norm_vector", header));
                        size_t inplane_rot = atoi(getStringFromHeader("implane_rotation", header).c_str());
                        std::string slice_orient = getStringFromHeader("slice_orientation", header);
                        
                        //Fallunterscheidung
                        // Wenn ((slice_orient == TRANSVERSE) gilt: (-45 < inplane_rot < 45)) ? -> COL : ROW -> phaseVec == col + readVec == row
                        // Wenn ((slice_orient != TRANSVERSE) gilt: (-45 < inplane_rot < 45)) ? -> ROW : COL -> phaseVec == row + readVec == col
                        std::string InPlanePhaseEncodingDirection;
                        if (0 == slice_orient.compare(0, slice_orient.length(), "TRANSVERSE")) {
                            InPlanePhaseEncodingDirection = (-45 < inplane_rot && inplane_rot < 45 ) ? "COL" : "ROW";
                        }
                        else {
                            InPlanePhaseEncodingDirection = (-45 < inplane_rot && inplane_rot < 45 ) ? "ROW" : "COL";
                        }

                        size_t fov_read = atoi(getStringFromHeader("fov_read", header).c_str());
                        size_t fov_phase = atoi(getStringFromHeader("fov_phase", header).c_str());
                        size_t slice_thickness = atoi(getStringFromHeader("slice_thickness", header).c_str());
                        size_t dimension_number = atoi(getStringFromHeader("dimension_number", header).c_str());
                        // get voxelGap out of the distance (in percent!) between slices
                        size_t distFactor = atoi(getStringFromHeader("distance_factor", header).c_str());
                        util::fvector4 voxelGap(0, 0, slice_thickness*(distFactor/100));
                        
                        //*********
                        
 						image_counter++;
												
						memcpy(dataBuffer + data_received, buffer, byteSize - data_received);
						data_received += byteSize - data_received;
						
						// ... do something ...
						/******************************/
					
                       if ( 0 == data_type.compare("short")){
                           char slice_buffer[width_slice * height_slice * sizeof(short)];
                            for(unsigned int _slice = 0; _slice < iim; _slice++) {
                                    for(unsigned int _row = 0; _row < height_slice; _row++) {
                                        char* line_start = dataBuffer + (sizeof(short) * (((_slice / slices_in_row) * (slices_in_row * height_slice * width_slice)) + 
                                                                                          (_row * width_slice * slices_in_row) + (_slice % slices_in_row * width_slice)));

                                        memcpy(slice_buffer + (_row * width_slice * sizeof(short)), line_start, (width_slice * sizeof(short)));
                                    }
                                
                                /********
                                * get each slice position from header 
                                */
                                std::string slice_pos = "slice_position_";
                                char buf[5];
                                sprintf(buf, "%i", _slice);
                                slice_pos.append(buf);
                                util::fvector4 slice_pos_vec = getVectorFromString(getStringFromHeader(slice_pos, header));//(val1, val2, val3);
                                //*********
                                
                                // now, create chunks per slice and feed it with metadata
                                data::Chunk ch(data::MemChunk<uint16_t>((uint16_t*)slice_buffer, height_slice,width_slice,1) );
				
                                ch.setPropertyAs("indexOrigin", slice_pos_vec);
                                ch.setPropertyAs<uint32_t>("acquisitionNumber", acq_nr);
                                //ch.setPropertyAs<>("acquisitionTime", acquisition_time);
                                if (true == moco){
                                    ch.setPropertyAs<uint16_t>("sequenceNumber", 0);
                                }
                                else {
                                    ch.setPropertyAs<uint16_t>("sequenceNumber", 1);
                                }
                                ch.setPropertyAs<std::string>("sequenceDescription", seq_descr);
                                

                                if ( 0 == InPlanePhaseEncodingDirection.compare(0, 3, "COL") ){
                                    ch.setPropertyAs<util::fvector4>("readVec", phase_vec);
                                    ch.setPropertyAs<util::fvector4>("phaseVec", read_vec);
                                    ch.setPropertyAs<util::fvector4>("voxelSize", util::fvector4(fov_read/width_slice,fov_phase/height_slice,slice_thickness,0));
                                }
                                else {
                                    ch.setPropertyAs<util::fvector4>("phaseVec", phase_vec);
                                    ch.setPropertyAs<util::fvector4>("readVec", read_vec);
                                    ch.setPropertyAs<util::fvector4>("voxelSize", util::fvector4(fov_phase/width_slice,fov_read/height_slice,slice_thickness,0));
                                }

                                
                                ch.setPropertyAs<util::fvector4>("sliceVec", slice_norm_vec);
                                ch.setPropertyAs<uint16_t>("repetitionTime",rep_time);
                                ch.setPropertyAs<std::string>("InPlanePhaseEncodingDirection",InPlanePhaseEncodingDirection);
                                ch.setPropertyAs<util::fvector4>( "voxelGap", util::fvector4() );
                                chunks.push_back(ch);
                            }
                       }
                        else {
                            printf("DATATYPE NOT SUPPORTED\n");
                        }

				
						
						/******************************/
						free(dataBuffer);
                        /*****************************/

                        
                        // evaluate the sending timing
                        // TODO:: remove this stuff sometime
                        if (true == moco){
                            timestampPreviousMOCO = timestampCurrentMOCO;
                            timestampCurrentMOCO = atof(getStringFromHeader("time_stamp", header).c_str());
                            double_t diffTimestamps = (timestampCurrentMOCO - timestampPreviousMOCO);
                            fprintf(timediffMOCO, "%.6lf\n", diffTimestamps );
                        }
                        if (false == moco){
                            timestampPrevious = timestampCurrent;
                            timestampCurrent = atof(getStringFromHeader("time_stamp", header).c_str());
                            double_t diffTimestamps = (timestampCurrent - timestampPrevious);
                            fprintf(timediffRECO, "%.6lf\n", diffTimestamps );
                        }
   					return 0;
					}
					
				}
                    
				
			}
				return 0;
			}
			
			void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
			}
			bool tainted()const {return false;}//internal plugins are not tainted
			
			~ImageFormat_SiemensTcpIp()
			{
                fclose(timediffRECO);
                fclose(timediffMOCO);
            }
			
			
			
		public:
			static int    sock;
			static struct sockaddr_in receiver_address;
			static int counter;
			static int image_counter;
			
			static unsigned int receiver_address_length;
			static bool firstHeaderArrived;
            static double_t timestampPrevious;
            static double_t timestampCurrent;
            static double_t timestampPreviousMOCO;
            static double_t timestampCurrentMOCO;
            static FILE* timediffMOCO;
            static FILE* timediffRECO;
            
		private:
            time_t startTime;
            time_t endTime;
			
            std::string getStringFromHeader(const std::string& propName, const std::string& header){
                
                std::string prop_start = "<";
                prop_start.append(propName);
                prop_start.append(">\n");
                std::string prop_end = "\n</";
                prop_end.append(propName);
                prop_end.append(">");
                std::string propString = header.substr((header.find(prop_start)+prop_start.length()), 
                                                         header.find(prop_end) - (header.find(prop_start)+prop_start.length()));
                return propString;
            }
            
            util::fvector4 getVectorFromString(std::string propName)
            {
                size_t indexK1 = propName.find(",", 0, 1);
                size_t indexK2 = propName.find(",", indexK1+1, 1);
                double_t val1 = atof(propName.substr(0, indexK1).c_str());
                double_t val2 = atof(propName.substr(indexK1+1, indexK2-indexK1).c_str());
                double_t val3 = atof(propName.substr(indexK2+1, propName.length()-indexK2).c_str());
                return util::fvector4(val1, val2, val3);
            }
			char   buffer[32768];
        };
		
		int    ImageFormat_SiemensTcpIp::sock;
		struct sockaddr_in ImageFormat_SiemensTcpIp::receiver_address;
		int ImageFormat_SiemensTcpIp::counter;
		int ImageFormat_SiemensTcpIp::image_counter;
		
		unsigned int ImageFormat_SiemensTcpIp::receiver_address_length;
		bool ImageFormat_SiemensTcpIp::firstHeaderArrived;
        double_t ImageFormat_SiemensTcpIp::timestampCurrent;
        double_t ImageFormat_SiemensTcpIp::timestampPrevious;
        double_t ImageFormat_SiemensTcpIp::timestampCurrentMOCO;
        double_t ImageFormat_SiemensTcpIp::timestampPreviousMOCO;
        FILE* ImageFormat_SiemensTcpIp::timediffMOCO;
        FILE* ImageFormat_SiemensTcpIp::timediffRECO;
        		
	}
}
isis::image_io::FileFormat *factory()
{
	isis::image_io::ImageFormat_SiemensTcpIp *pluginRtExport = new isis::image_io::ImageFormat_SiemensTcpIp();
	
	pluginRtExport->sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("[receiver2] sock --> %i\n", pluginRtExport->sock);
	
	memset((void*)&pluginRtExport->receiver_address, 0, sizeof(pluginRtExport->receiver_address));
	pluginRtExport->receiver_address.sin_family      = PF_INET;
	pluginRtExport->receiver_address.sin_addr.s_addr = INADDR_ANY;
	pluginRtExport->receiver_address.sin_port        = htons(54321);
		
	printf("[bind] --> %i\n", bind(pluginRtExport->sock, (struct sockaddr*)&pluginRtExport->receiver_address, sizeof(pluginRtExport->receiver_address)));
		
	pluginRtExport->receiver_address_length = sizeof(pluginRtExport->receiver_address);
	pluginRtExport->counter = 0;
	pluginRtExport->image_counter = 0;
	
	pluginRtExport->timestampCurrent = 0;
        pluginRtExport->timestampPrevious = 0;
        pluginRtExport->timestampCurrentMOCO = 0;
        pluginRtExport->timestampPreviousMOCO = 0;
        pluginRtExport->timediffMOCO = fopen("/tmp/timediffMOCO.txt", "w");
	pluginRtExport->timediffRECO = fopen("/tmp/timediffRECO.txt", "w");
	
	
	return (isis::image_io::FileFormat*) pluginRtExport;
}

