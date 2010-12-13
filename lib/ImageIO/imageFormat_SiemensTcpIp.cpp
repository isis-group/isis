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
				data_received = 0L;
				sprintf(header_start, "<data_block_header>");
				sprintf(header_end, "</data_block_header>");
				int byteSize = 294912;
				
				for(;;){
				memset(buffer, 0, sizeof(buffer));
				length = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&receiver_address, &receiver_address_length);
				if(length == 0) {
					exit(0);
				}
				printf("[%i] Received %i bytes\n", ++counter, length);
				printf("buffer: %s\n", buffer);
   			    printf("header_start: %s\n", header_start);
				
				
				if(memcmp(buffer, header_start, sizeof(header_start) - 1) == 0) {
					firstHeaderArrived = true;
					printf("[receiver2] received block with header\n");
					data_received = 0;
					int header_length = 0;
					while(memcmp(header_end, (buffer + header_length), sizeof(header_end)) != 0) {
						header_length++;
					}
					header_length += sizeof(header_end);
					char header[header_length];
					memcpy(header, buffer, header_length);
					
					char* byteSizeStringStart = strstr(header, "<data_size_in_bytes>") + 20 ;
					char* byteSizeStringEnd   = strstr(header, "</data_size_in_bytes>");
					
					char byteSizeString[byteSizeStringEnd - byteSizeStringStart];
					strncat(byteSizeString, byteSizeStringStart, byteSizeStringEnd - byteSizeStringStart);
					
					//byteSize = strtol(byteSizeString, NULL, 10);
					//byteSize = atoi(byteSizeString);
					sscanf(byteSizeString, "%i", &byteSize);
					printf("byteSize: %s %i\n", byteSizeString, byteSize);
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
						image_counter++;
						
						//char filename[64];
						//sprintf(filename, "/Users/torsten/Data/image%04i.img", image_counter);
						//FILE *image_file = fopen(filename, "w+");
						//fwrite(dataBuffer, byteSize, 1, image_file);
						//fclose(image_file);
						
						memcpy(dataBuffer + data_received, buffer, byteSize - data_received);
						data_received += byteSize - data_received;
						printf("[receiver2] did receive %i bytes of data\n", byteSize);
						
						// ... do something ...
						/******************************/
					
						uint nrRows = 384;
						uint nrCols = 384;
						uint nrSlices = 1;
						boost::shared_ptr<data::Chunk> ch(new data::MemChunk<uint16_t>((uint16_t*)dataBuffer, nrCols,nrRows,nrSlices) );
						ch->setProperty("indexOrigin", util::fvector4(1,2,image_counter));
						ch->setProperty<uint32_t>("acquisitionNumber", image_counter);
						ch->setProperty<uint16_t>("sequenceNumber", 1);
						ch->setProperty("readVec", util::fvector4(1,0,0,0));
						ch->setProperty("phaseVec", util::fvector4(0,1,0,0));
						ch->setProperty("voxelSize", util::fvector4(1,1,1,0));
						ch->setProperty("repetitionTime",2000);
						ch->setProperty<std::string>("InPlanePhaseEncodingDirection","COL");
						ch->setProperty( "voxelGap", util::fvector4() );
						chunks.push_back(ch);
						
					//switch ('DATENTYP') {
//						case UINT8_C:
//							retChunk.reset(new Chunk()); 
//							break;
//							
//						default:
//							break;
//					}	
				
						
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
			int    length;
			char* dataBuffer;
			long data_received;
			char header_start[19];
			char header_end[20];
			
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

//Thread stuff
//for(;;){
//	memset(buffer, 0, sizeof(buffer));
//	length = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&receiver_address, &receiver_address_length);
//	if(length == 0) {
//		exit(0);
//	}
//	printf("[%i] Received %i bytes\n", ++counter, length);
//	
//	//	int index;
//	
//	//for(index=0; index<32768; index++) {
//	//	printf("%0X %c\n", buffer[index], buffer[index]);
//	//}
//	
//	
//	//char start[20];
//	
//	//memcpy(start, buffer, 20);
//	
//	//printf("start --> %s\n", start);
//	
//	if(memcmp(buffer, header_start, sizeof(header_start) - 1) == 0) {
//		printf("[receiver2] received block with header\n");
//		data_received = 0;
//		int header_length = 0;
//		while(memcmp(header_end, (buffer + header_length), sizeof(header_end)) != 0) {
//			header_length++;
//		}
//		header_length += sizeof(header_end);
//		char header[header_length];
//		memcpy(header, buffer, header_length);
//		//			printf("[receiver2] header of length %i\n%s\n", header_length, header);
//		
//		char* byteSizeStringStart = strstr(header, "<data_size_in_bytes>") + 20 ;
//		char* byteSizeStringEnd   = strstr(header, "</data_size_in_bytes>");
//		
//		//			printf("[receiver2]  byteSizeStringStart %s\n", byteSizeStringStart);
//		//			printf("[receiver2]  byteSizeStringEnd %s\n", byteSizeStringEnd);
//		
//		
//		char byteSizeString[byteSizeStringEnd - byteSizeStringStart];
//		strncat(byteSizeString, byteSizeStringStart, byteSizeStringEnd - byteSizeStringStart);
//		
//		//			printf("[receiver2] will receive %s bytes of data\n", byteSizeString);
//		
//		
//		byteSize = atoi(byteSizeString);
//		
//		data = malloc(byteSize);
//		
//		// copy the first bit of data
//		memcpy(data, (buffer + header_length), sizeof(buffer) - header_length);
//		
//		data_received += sizeof(buffer) - header_length;
//		
//	} else {
//		
//		if(byteSize - data_received > 32768) {
//			memcpy(data + data_received, buffer, sizeof(buffer));
//			data_received += sizeof(buffer);
//			//printf("[receiver2] did receive %i bytes of data so far\n", data_received);
//		} else {
//			// image complete
//			image_counter++;
//			
//			char filename[64];
//			sprintf(filename, "/Users/torsten/Data/image%04i.img", image_counter);
//			FILE *image_file = fopen(filename, "w+");
//			fwrite(data, byteSize, 1, image_file);
//			fclose(image_file);
//			
//			memcpy(data + data_received, buffer, byteSize - data_received);
//			data_received += byteSize - data_received;
//			printf("[receiver2] did receive %i bytes of data\n", byteSize);
//			
//			// ... do something ...
//			
//			free(data);
//		}
//		
//	}
//	
//}
