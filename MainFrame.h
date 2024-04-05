#pragma once
#include <wx/wx.h>
#include <fstream>

enum
{
	ID_CONVERT_GRAYSCALE = wxID_HIGHEST + 1,
	ID_CONVERT_AUTO_LEVEL,
	ID_CONVERT_HUFFMAN,
	ID_CONVERT_ORDERED_DITHERING,
	ID_CONVERT_MIRROR,
};

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);

private:

	// Variables to store the state of the image
	bool is_grayscale_{ false }, is_dithered_{ false }, is_auto_leveled_{ false };
	wxImage original_image_, grayscale_image_;
	wxString file_path_;
	wxBoxSizer* sizer_;
	wxBitmap m_bitmap, m_transformed_bitmap_;
	wxStaticBitmap* m_bitmapCtrl, * m_tBitmapCtrl;
	std::vector<uint8_t> image_data_;

	// Standard Bitmap header structure
	#pragma pack(push, 1) // Ensure structure is packed
	struct BMPHeader {
		uint16_t signature;  // File signature "BM"
		uint32_t fileSize;   // File size
		uint16_t reserved1;  // Reserved, must be zero
		uint16_t reserved2;  // Reserved, must be zero
		uint32_t dataOffset; // Offset to the beginning of pixel data
		uint32_t headerSize; // Size of the header (40 bytes)
		int32_t width;       // Image width
		int32_t height;      // Image height
		uint16_t planes;     // Number of color planes (must be 1)
		uint16_t bitsPerPixel; // Bits per pixel
		uint32_t compression; // Compression method (0 for uncompressed)
		uint32_t imageSize;   // Size of the image data
		int32_t xPixelsPerMeter; // Horizontal resolution (pixels per meter)
		int32_t yPixelsPerMeter; // Vertical resolution (pixels per meter)
		uint32_t colorsUsed;      // Number of colors in the palette (usually ignored)
		uint32_t importantColors; // Number of important colors (usually ignored)
	};
	#pragma pack(pop)

	// Open file event handler
	void OnOpenFile(wxCommandEvent& event);

	void LoadBitmapFromFile();

	// Main application exit event handler
	void OnExit(wxCommandEvent& event);

	// Handle the event when grayscale conversion is requested
	void OnGrayScale(wxCommandEvent& event);

	// Convert the image to grayscale
	static void ConvertImageToGrayScale(wxImage& image);

	// Handle the event when dithering is requested
	void OnDitherButtonClick(wxCommandEvent& event);

	// Perform ordered dithering on a wxImage using a 4x4 Bayer dither matrix
	void OrderedDither(wxImage& image, const int matrix_size);

	// On Auto Level button click
	void OnAutoLevelButtonClick(wxCommandEvent& event);

	static void AutoLevel(wxImage& image);

	// On Huffman button click
	void OnHuffmanButtonClick(wxCommandEvent& event);

	void HuffmanEncode();

	
	inline std::vector<uint8_t> wxImageToVector(wxImage& image, const size_t width, const size_t height);

	/*
	 * Optional operations and dependencies
	 */

	static void mirror(std::vector<uint8_t>& image_data, const size_t width, const size_t height);
};