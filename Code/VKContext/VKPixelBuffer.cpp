// VKPixelBuffer.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKPixelBuffer.h"

extern "C" {
	#include <zip.h>
	#include <png.h>
	#include <jpeglib.h>

	#define INPUT_BUF_SIZE 4096
	typedef struct { 
		struct jpeg_source_mgr pub; 
		zip *pZip;
		zip_file *pFile;
		unsigned char szBuffer[INPUT_BUF_SIZE];
	} zip_jpeg_source;

	void zip_init_source(j_decompress_ptr cinfo) { 
		zip_jpeg_source *src = (zip_jpeg_source *)cinfo->src; 
	} 
	boolean zip_fill_input_buffer(j_decompress_ptr cinfo) { 
		zip_jpeg_source *src = (zip_jpeg_source *)cinfo->src;
		src->pub.next_input_byte = src->szBuffer;
		src->pub.bytes_in_buffer = zip_fread(src->pFile, src->szBuffer, INPUT_BUF_SIZE);
		return src->pub.bytes_in_buffer > 0 ? TRUE : FALSE;
	}
	void zip_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
		zip_jpeg_source *src = (zip_jpeg_source *)cinfo->src;
		if(num_bytes > 0) {
			while(num_bytes > (long) src->pub.bytes_in_buffer) {
				num_bytes -= (long) src->pub.bytes_in_buffer;
				(void)zip_fill_input_buffer(cinfo);
			}
			src->pub.next_input_byte += num_bytes;
			src->pub.bytes_in_buffer -= num_bytes;
		}
	}
	void zip_term_source(j_decompress_ptr cinfo) {
		zip_jpeg_source *src = (zip_jpeg_source *) cinfo->src;
	}

	void zip_jpeg_memory_src(j_decompress_ptr cinfo, zip *pZip, zip_file *pFile) {
		if (cinfo->src == NULL)
			cinfo->src = (struct jpeg_source_mgr *)(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT, sizeof(zip_jpeg_source)); 
		zip_jpeg_source *src = (zip_jpeg_source *)cinfo->src; 
		src->pZip = pZip;
		src->pFile = pFile;
		src->pub.init_source = zip_init_source;
		src->pub.fill_input_buffer = zip_fill_input_buffer;
		src->pub.skip_input_data = zip_skip_input_data;
		src->pub.resync_to_restart = jpeg_resync_to_restart;
		src->pub.term_source = zip_term_source;
		src->pub.bytes_in_buffer = 0;
		src->pub.next_input_byte = NULL;
	}
};


namespace VK {

// Force specific instantiations to be built (allows the methods to be defined in the cpp)
template class PixelBuffer<unsigned char>;
//template class PixelBuffer<char>;
//template class PixelBuffer<short>;
//template class PixelBuffer<unsigned short>;
//template class PixelBuffer<int>;
//template class PixelBuffer<unsigned int>;
//template class PixelBuffer<float>;
//#ifdef VK_DOUBLE
//template class PixelBuffer<double>;
//#endif

static bool g_bJPGError = false;
static void jpg_error_exit(j_common_ptr cinfo) {
	char buffer[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, buffer);
	VKLogError("JPEG error: %s", buffer);
	g_bJPGError = true;
}

bool Load(PixelBuffer<uint8_t> &pb, const char *pszFile) {
	std::string extn = VK::Path(pszFile).extension();
	_strlwr((char *)extn.c_str());
	if (extn == "ppm")
		return LoadPPM(pb, pszFile, 0, 0, 0);
	else if (extn == "raw")
		return LoadRAW(pb, pszFile, 0, 0, 0);
	else if (extn == "png")
		return LoadPNG(pb, pszFile, 0, 0, 0);
	else if (extn == "jpg" || extn == "jpeg")
		return LoadJPG(pb, pszFile, 0, 0, 0);
	return false;
}

bool LoadPPM(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	char header[256], *cPtr = NULL, *tmp = NULL;
	FILE *fPtr = fopen(pszFile, "rb");

	if (!fPtr) {
		VKLogError("Unable to open %s.", pszFile);
		return false;
	}

	cPtr = fgets(header, 256, fPtr); // P6
	if (cPtr == NULL || strncmp(header, "P6\n", 3)) {
		fclose(fPtr);
		return false;
	}

	do {
		cPtr = fgets(header, 256, fPtr);
		if (cPtr == NULL) {
			fclose(fPtr);
			return false;
		}
	} while (!strncmp(header, "#", 1));

	uint32_t w = 0, h = 0, c = 3;
	sscanf(header, "%u %u", &h, &w);
	if ((nWidth != 0 && w != nWidth) ||
		(nHeight != 0 && h != nHeight) ||
		(nChannels != 0 && c != nChannels))
		return false;

	tmp = fgets(header, 256, fPtr); // Format
	(void)tmp;
	if (cPtr == NULL || strncmp(header, "255\n", 3)) {
		fclose(fPtr);
		return false;
	}

	pb.create(w, h, 1, c);
	uint8_t *ptr = pb.getBuffer();
	size_t s = fread(pb.getBuffer(), 1, w * h * 3, fPtr);
	fclose(fPtr);
	return true;
}


//template <>
//bool PixelBuffer<unsigned char>::loadJPG(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
bool LoadJPG(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	VKLogDebug("Loading %s", pszFile);

	// Initialize the JPEG struct
	g_bJPGError = false;
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpg_error_exit;
	jpeg_create_decompress(&cinfo);

	// Currently only supports 8-bit RGB and RGBA
#ifdef ANDROID
	zip *pZip = ::zip_open(Path::APK(), 0, NULL);
	if(!pZip) {
		VKLogError("Failed to open apk: ", (const char *)Path::APK());
		return false;
	}
	zip_file *pFile = zip_fopen(pZip, pszFile, 0);
	if(!pFile) {
		VKLogError("Failed to open file in apk: ", pszFile);
		return false;
	}
	zip_jpeg_memory_src(&cinfo, pZip, pFile);
#else
	FILE *pFile = fopen(pszFile, "rb");
	if(pFile == NULL) {
		VKLogError("Unable to open %s.", pszFile);
		return false;
	}
	jpeg_stdio_src(&cinfo, pFile);
#endif

	jpeg_read_header(&cinfo, TRUE);
	if((nWidth != 0 && cinfo.image_width != nWidth) ||
	   (nHeight != 0 && cinfo.image_height != nHeight) ||
	   (nChannels != 0 && cinfo.num_components != nChannels))
		return false;

	pb.create(cinfo.image_width, cinfo.image_height, 1, cinfo.num_components);
	std::vector<JSAMPROW> row(pb.getHeight());
	for(int i=0; i<pb.getHeight(); i++)
		row[i] = (JSAMPROW)pb.operator()(0, pb.getHeight()-1-i);
	
	// Flush the image to the JPEG file
	jpeg_start_decompress(&cinfo);
	for(int i=0; i < pb.getHeight(); )
		i += jpeg_read_scanlines(&cinfo, &row[i], pb.getHeight()-i);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	// Clean up
#ifdef ANDROID
		zip_fclose(pFile);
		zip_close(pZip);
#else
		fclose(pFile);
#endif
	return !g_bJPGError;
}

void pngZipRead(png_struct *png_ptr, png_byte *outBytes, png_size_t byteCountToRead) {
	zip_file *pFile = (zip_file *)png_get_io_ptr(png_ptr);
	int n = zip_fread(pFile, outBytes, byteCountToRead);
	if(n != byteCountToRead)
		VKLogException("Woops");
}

//template <>
//bool PixelBuffer<unsigned char>::loadPNG(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
bool LoadPNG(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	VKLogDebug("Loading %s", pszFile);
	// Currently only supports 8-bit RGB and RGBA
#ifdef ANDROID
	zip *pZip = ::zip_open(Path::APK(), 0, NULL);
	if(!pZip) {
		VKLogError("Failed to open apk: ", (const char *)Path::APK());
		return false;
	}
	zip_file *pFile = zip_fopen(pZip, pszFile, 0);
	if(!pFile) {
		VKLogError("Failed to open file in apk: ", pszFile);
		return false;
	}
#else
	FILE *pFile = fopen(pszFile, "rb");
	if(pFile == NULL) {
		VKLogError("Unable to open %s.", pszFile);
		return false;
	}
#endif

	unsigned char header[8];
#ifdef ANDROID
	zip_fread(pFile, header, 8);
#else
	fread(header, 1, 8, pFile);
#endif
	if(png_sig_cmp(header, 0, 8)) {
		VKLogError("%s is not a valid PNG file.", pszFile);
#ifdef ANDROID
		zip_fclose(pFile);
		zip_close(pZip);
#else
		fclose(pFile);
#endif
		return false;
	}

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;

	if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
		VKLogError("png_create_read_struct failed.");
#ifdef ANDROID
		zip_fclose(pFile);
		zip_close(pZip);
#else
		fclose(pFile);
#endif
		return false;
	}

	if((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
		VKLogError("png_create_info_struct failed.");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
#ifdef ANDROID
		zip_fclose(pFile);
		zip_close(pZip);
#else
		fclose(pFile);
#endif
		return false;
	}

	setjmp(png_jmpbuf(png_ptr));
#ifdef ANDROID
	png_set_read_fn(png_ptr, pFile, pngZipRead);
#else
	png_init_io(png_ptr, pFile);
#endif
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	uint8_t color_type = png_get_color_type(png_ptr, info_ptr);
	uint8_t bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	uint8_t channels =
		color_type == PNG_COLOR_TYPE_RGB && bit_depth == 8 ? 3 :
		color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth == 8 ? 4 :
		0;

	if(channels == 0) {
		VKLogError("Attempting to read an unsupported format from %s.", pszFile);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
#ifdef ANDROID
		zip_fclose(pFile);
		zip_close(pZip);
#else
		fclose(pFile);
#endif
		return false;
	}

	if((nWidth != 0 && (uint16_t)png_get_image_width(png_ptr, info_ptr) != nWidth) ||
	   (nHeight != 0 && (uint16_t)png_get_image_height(png_ptr, info_ptr) != nHeight) ||
	   (nChannels != 0 && channels != nChannels)) {
#ifdef ANDROID
		zip_fclose(pFile);
		zip_close(pZip);
#else
		fclose(pFile);
#endif
		return false;
	}

	pb.create((uint16_t)png_get_image_width(png_ptr, info_ptr), (uint16_t)png_get_image_height(png_ptr, info_ptr), 1, channels);

	int nRowSize = pb.getWidth() * pb.getChannels();
	png_read_update_info(png_ptr, info_ptr);
	setjmp(png_jmpbuf(png_ptr));
	row_pointers = new png_bytep[pb.getHeight()];
	for(int y=0; y<pb.getHeight(); y++)
		row_pointers[y] = (unsigned char *)pb.getBuffer() + (pb.getHeight()-1-y) * nRowSize;
	png_read_image(png_ptr, row_pointers);
	delete row_pointers;

	png_destroy_read_struct(&png_ptr, NULL, NULL);
#ifdef ANDROID
	zip_fclose(pFile);
	zip_close(pZip);
#else
	fclose(pFile);
#endif
	return true;
}

//template <class T>
//bool PixelBuffer<T>::loadRAW(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
bool LoadRAW(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	pb.create(nWidth, nHeight, 1, nChannels);
	std::ifstream in(pszFile, std::ios::binary);
	in.seekg(0, std::ios::end);
	if((int)in.tellg() != pb.getBufferSize())
		return false;
	in.seekg(0, std::ios::beg);
	in.read((char *)pb.getBuffer(), pb.getBufferSize());
	if(!in)
		return false;
	return true;
}

//template <class T>
//bool PixelBuffer<T>::saveJPG(const char *pszFile) {
bool SaveJPG(PixelBuffer<uint8_t> &pb, const char *pszFile) {
	// Currently only supports 8-bit RGB
	if(pb.getChannels() != 3) {
		VKLogError("Attempting to write an unsupported format to %s.", pszFile);
		return false;
	}

	// Open the file we're dumping to
	FILE *pFile;
	pFile = fopen(pszFile, "wb");
	if(pFile == NULL)
		return false;

	std::vector<JSAMPROW> row(pb.getHeight());
	for(int i=0; i<pb.getHeight(); i++)
		row[i] = (JSAMPROW)pb(0, pb.getHeight()-1-i);

	// Initialize the JPEG struct
	g_bJPGError = false;
	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpg_error_exit;
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, pFile);
	cinfo.image_width = pb.getWidth();
	cinfo.image_height = pb.getHeight();
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	const int JPEG_QUALITY = 95;
	jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE);

	// Flush the image to the JPEG file
	jpeg_start_compress(&cinfo, TRUE);
	jpeg_write_scanlines(&cinfo, &row[0], pb.getHeight());
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	// Clean up
	fclose(pFile);
	return !g_bJPGError;
}

//template <class T>
//bool PixelBuffer<T>::savePNG(const char *pszFile)
bool SavePNG(PixelBuffer<uint8_t> &pb, const char *pszFile) {
	// Currently only supports 8-bit RGB and RGBA
	if(pb.getChannels() != 3 && pb.getChannels() != 4) {
		VKLogError("Attempting to write an unsupported format to %s.", pszFile);
		return false;
	}

	FILE *pFile = fopen(pszFile, "wb");
	if(pFile == NULL) {
		VKLogError("Unable to create %s.", pszFile);
		return false;
	}

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;

	if((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
		VKLogError("png_create_write_struct failed.");
		return false;
	}
	if((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
		VKLogError("png_create_info_struct failed.");
		return false;
	}
	setjmp(png_jmpbuf(png_ptr));
	png_init_io(png_ptr, pFile);
	png_set_IHDR(png_ptr, info_ptr, pb.getWidth(), pb.getHeight(), 8, pb.getChannels() == 4 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	int nRowSize = pb.getChannels() * pb.getWidth();
	row_pointers = new png_bytep[pb.getHeight()];
	for(int y=0; y<pb.getHeight(); y++)
		row_pointers[y] = (unsigned char *)pb.getBuffer() + (pb.getHeight()-1-y) * nRowSize;
	png_write_image(png_ptr, row_pointers);
	delete row_pointers;

	png_write_end(png_ptr, NULL);
	fclose(pFile);
	return true;
}

} // namespace VK
