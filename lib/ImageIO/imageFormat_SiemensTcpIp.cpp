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
			std::string name()const {
				return "SiemensTcpIp";
			}
			
			int load ( data::ChunkList &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
				
				firstHeaderArrived = false;
				unsigned long data_received = 0L;
                std::string header_start = "<data_block_header>";
                std::string header_end =  "</data_block_header>";
                unsigned int byteSize = 0;
                char* dataBuffer;
                std::string header;
             	
				for(;;){
				memset(buffer, 0, sizeof(buffer));
				unsigned int length = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&receiver_address, &receiver_address_length);
				if(length == 0) {
					exit(0);
				}
				
				
				if(memcmp(buffer, header_start.c_str(), sizeof(header_start) - 1) == 0) {
					firstHeaderArrived = true;
					printf("[receiver2] received block with header\n");
					data_received = 0L;
					unsigned int header_length = 0;
					while(memcmp(header_end.c_str(), (buffer + header_length), sizeof(header_end)) != 0) {
						header_length++;
					}
					header_length += sizeof(header_end);
                    header = std::string(buffer, header_length);
                    
                    size_t byteSizeStringStart = header.find("<data_size_in_bytes>") +20 ;
                    size_t byteSizeStringEnd   = header.find("</data_size_in_bytes>");
                    
                    std::string byteSizeString = header.substr(byteSizeStringStart, byteSizeStringEnd - byteSizeStringStart);
                    byteSize = atoi(byteSizeString.c_str());
					
					printf("byteSize: '%s' '%i'\n", byteSizeString.c_str(), byteSize);
					dataBuffer = (char*)malloc(byteSize);
					
					// copy the first bit of data
					memcpy(dataBuffer, (buffer + header_length), sizeof(buffer) - header_length);
					
					data_received += sizeof(buffer) - header_length;
					
				} else if (true == firstHeaderArrived) {
					
                   if(byteSize - data_received > 32768) {
						memcpy(dataBuffer + data_received, buffer, sizeof(buffer));
						data_received += sizeof(buffer);
					} else {
						// image complete
                        std::string width_start = "<width>";
                        std::string width_end   = "</width>";
                        size_t width = atoi(header.substr((header.find(width_start)+width_start.length()), header.find(width_end) - (header.find(width_start)+width_start.length())).c_str());
                        std::string height_start = "<height>";
                        std::string height_end   = "</height>";
                        
                        std::string moco_start = "<motion_corrected>";
                        std::string moco_end   = "</motion_corrected>";
                        bool moco = header.substr((header.find(moco_start)+moco_start.length()), header.find(moco_end) - (header.find(moco_start)+moco_start.length())).compare("yes") == 0 ? true : false;
                        
                        
                        size_t height = atoi(header.substr((header.find(height_start)+height_start.length()), header.find(height_end) - (header.find(height_start)+height_start.length())).c_str());
                        std::string mosaic_start = "<mosaic>";
                        std::string mosaic_end   = "</mosaic>";
                        bool mosaic = header.substr((header.find(mosaic_start)+mosaic_start.length()), header.find(mosaic_end) - (header.find(mosaic_start)+mosaic_start.length())).compare("yes") == 0 ? true : false;
                        size_t iim = 1;
                        printf("Header \n %s \n", header.c_str());
                        if ( true == mosaic){
                            std::string iim_start = "<images_in_mosaic>";
                            std::string iim_end   = "</images_in_mosaic>";
                            iim = atoi(header.substr((header.find(iim_start)+iim_start.length()), header.find(iim_end) - (header.find(iim_start)+iim_start.length())).c_str());
                        }
                        
                        std::string type_start = "<data_type>";
                        std::string type_end   = "</data_type>";
                        std::string data_type = header.substr((header.find(type_start)+type_start.length()), header.find(type_end) - (header.find(type_start)+type_start.length()));
                        
                        // Mosaics are always quadratic, so don't bother but only looking for the rows
                        size_t slices_in_row = static_cast<size_t> (ceil(sqrt(static_cast<double_t> (iim) )));
                        size_t width_slice = width / slices_in_row;
                        size_t height_slice = height / slices_in_row;
                        
                        printf("width_slices: %ld height_slices: %ld\n", width_slice, height_slice);
                        printf("data_type %s\n", data_type.c_str());
                        
                        std::string acq_nr_start = "<acquisition_number>";
                        std::string acq_nr_end   = "</acquisition_number>";
                        size_t acq_nr = atoi(header.substr((header.find(acq_nr_start)+acq_nr_start.length()), header.find(acq_nr_end) - (header.find(acq_nr_start)+acq_nr_start.length())).c_str());
                        
                        std::string seq_descr_start = "<sequence_description>";
                        std::string seq_descr_end   = "</sequence_description>";
                        std::string seq_descr = header.substr((header.find(seq_descr_start)+seq_descr_start.length()), header.find(seq_descr_end) - (header.find(seq_descr_start)+seq_descr_start.length()));
                        
                        
						image_counter++;
												
						memcpy(dataBuffer + data_received, buffer, byteSize - data_received);
						data_received += byteSize - data_received;
						printf("[receiver2] did receive %i bytes of data\n", byteSize);
						
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
                                
                                boost::shared_ptr<data::Chunk> ch(new data::MemChunk<uint16_t>((uint16_t*)slice_buffer, height_slice,width_slice,1) );
                                ch->setProperty("indexOrigin", util::fvector4(0,0,_slice));
                                ch->setProperty<uint32_t>("acquisitionNumber", acq_nr);
                                if (true == moco){
                                    ch->setProperty<uint16_t>("sequenceNumber", 2);
                                }
                                else {
                                    ch->setProperty<uint16_t>("sequenceNumber", 1);
                                }
                                ch->setProperty<std::string>("sequenceDescription", seq_descr);
                                

                                ch->setProperty("readVec", util::fvector4(1,0,0,0));
                                ch->setProperty("phaseVec", util::fvector4(0,-1,0,0));
                                ch->setProperty("voxelSize", util::fvector4(3,3,6,0));
                                ch->setProperty("repetitionTime",2000);
                                ch->setProperty<std::string>("InPlanePhaseEncodingDirection","COL");
                                ch->setProperty( "voxelGap", util::fvector4() );
                                chunks.push_back(ch);
                            }
                       }
                        else {
                            printf("DATATYPE NOT SUPPORTED\n");
                        }

				
						
						/******************************/
						free(dataBuffer);
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
			{}
			
			
			
		public:
			static int    sock;
			static struct sockaddr_in receiver_address;
			static int counter;
			static int image_counter;
			
			static unsigned int receiver_address_length;
			static bool firstHeaderArrived;
		private:
			
			char   buffer[32768];
        
        };
		
		int    ImageFormat_SiemensTcpIp::sock;
		struct sockaddr_in ImageFormat_SiemensTcpIp::receiver_address;
		int ImageFormat_SiemensTcpIp::counter;
		int ImageFormat_SiemensTcpIp::image_counter;
		
		unsigned int ImageFormat_SiemensTcpIp::receiver_address_length;
		bool ImageFormat_SiemensTcpIp::firstHeaderArrived;
		
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
	pluginRtExport->receiver_address.sin_port        = htons(12345);
		
	printf("[bind] --> %i\n", bind(pluginRtExport->sock, (struct sockaddr*)&pluginRtExport->receiver_address, sizeof(pluginRtExport->receiver_address)));
		
	pluginRtExport->receiver_address_length = sizeof(pluginRtExport->receiver_address);
	pluginRtExport->counter = 0;
	pluginRtExport->image_counter = 0;
	
	
	
	
	return (isis::image_io::FileFormat*) pluginRtExport;
}

