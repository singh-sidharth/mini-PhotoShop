#include "MainFrame.h"
#include <wx/wx.h>

#include "HuffmanEncoder.h"

MainFrame::MainFrame(const wxString& title)
	: wxFrame(nullptr, wxID_ANY, title),
	file_path_(wxString("image1.bmp")),
	sizer_(new wxBoxSizer(wxHORIZONTAL)),
	m_bitmap(wxNullBitmap),
	m_transformed_bitmap_(wxNullBitmap)
{
	// Create a panel to hold the widgets
	const auto panel = new wxPanel(this, wxID_ANY);

	panel->SetBackgroundColour(wxColour(159, 159, 159));


	m_bitmapCtrl = new wxStaticBitmap(panel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	m_tBitmapCtrl = new wxStaticBitmap(panel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

	// Loading default image
	LoadBitmapFromFile();

	// Create a menu bar
	const auto file_menu = new wxMenu;

	file_menu->Append(wxID_OPEN, "&Open\tCtrl-O", "Open a Bitmap file");
	Bind(wxEVT_MENU, &MainFrame::OnOpenFile, this, wxID_OPEN);

	file_menu->Append(ID_CONVERT_ORDERED_DITHERING, wxT("O&rdered Dithering\tCtrl-R"),
		"Shows the gray image (of the original image) and its ordered dithering version side-by-side");
	Bind(wxEVT_MENU, &MainFrame::OnDitherButtonClick, this, ID_CONVERT_ORDERED_DITHERING);

	file_menu->Append(ID_CONVERT_GRAYSCALE, wxT("&Grayscale\tCtrl-G"),
		"Shows the original image and its gray image side by side");
	Bind(wxEVT_MENU, &MainFrame::OnGrayScale, this, ID_CONVERT_GRAYSCALE);

	file_menu->Append(ID_CONVERT_AUTO_LEVEL, wxT("Auto &Level\tCtrl-L"),
		"Shows the original image and its auto leveled image side by side");
	Bind(wxEVT_MENU, &MainFrame::OnAutoLevelButtonClick, this, ID_CONVERT_AUTO_LEVEL);

	file_menu->Append(ID_CONVERT_HUFFMAN, wxT("&Huffman\tCtrl-H"), "Shows the entropy of the gray image");
	Bind(wxEVT_MENU, &MainFrame::OnHuffmanButtonClick, this, ID_CONVERT_HUFFMAN);

	file_menu->Append(wxID_EXIT, wxT("&Quit\tCtrl-Q"), wxT("Exit this program"));
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);

	// Optional operations
	const auto optional_menu = new wxMenu;
	optional_menu->Append(wxID_ABOUT, wxT("&About"));

	// Add the menu bar to the frame
	const auto menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&Core Operations"));
	menu_bar->Append(optional_menu, wxT("O&ptional Operations"));
	MainFrame::SetMenuBar(menu_bar);

	// Add the original image to the sizer
	sizer_->Add(m_bitmapCtrl, 1, wxEXPAND | wxALL, 5);
	panel->SetSizerAndFit(sizer_);

	// Create a status bar
	MainFrame::CreateStatusBar();
	MainFrame::SetStatusText(wxT("Welcome to mini Photoshop by Sidharth!"));
}

void MainFrame::OnOpenFile(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Open Image file"), "", "", "Image files (*.bmp)|*.bmp", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	file_path_ = openFileDialog.GetPath();
	//Start loading the file
	LoadBitmapFromFile();
}

// Custom implementation of the BMP Parsing
// We further utilize this to convert the vector to wxImage and then to wxBitmap
void MainFrame::LoadBitmapFromFile()
{

	//reset all flags
	is_grayscale_ = false;
	is_auto_leveled_ = false;
	is_dithered_ = false;

	// Reset layouts and images
	m_tBitmapCtrl->SetBitmap(wxNullBitmap);
	sizer_->Detach(m_tBitmapCtrl);
	sizer_->Layout();
	original_image_ = grayscale_image_ = wxNullImage;

	std::ifstream file(file_path_.ToStdWstring(), std::ios::binary);

	if (!file)
	{
		wxLogError("Failed to open file: %s", file_path_);
		return;
	}

	// Read BMP header
	//char header[54];
	BMPHeader header;
	file.read(reinterpret_cast<char*>(&header), 54);

	// Check if it's a BMP file
	if (header.signature != 0x4D42) { // Check for "BM" signature
		wxLogError("Not a BMP file.");
		return;
	}

	if (header.bitsPerPixel != 24 || header.compression != 0) {
		wxLogError("Unsupported BMP format.");
		return;
	}

	// Get image dimensions
	const int width = header.width;
	const int height = std::abs(header.height); // Height might be negative (for bottom-to-top image)

	// Allocate memory for image data
	

	// Get the size of the file
	file.seekg(header.dataOffset); // Seek to the beginning of pixel data

	const int rowSize = ((header.width * 3 + 3) & ~3); // Width * 3 bytes per pixel (RGB), rounded up to a multiple of 4
	const int padding = rowSize - header.width * 3; // Padding bytes at end of each row

	std::vector<unsigned char> image_data(rowSize * height);

	for (int y = height - 1; y >= 0; --y) { // Read rows from bottom to top
		file.read(reinterpret_cast<char*>(&image_data[y * rowSize]), header.width * 3);
		file.seekg(padding, std::ios::cur); // Skip padding bytes
	}

	// Convert BGR to RGB
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			std::swap(image_data[(y * rowSize) + (x * 3)], image_data[(y * rowSize) + (x * 3) + 2]);
		}
	}

	// Close file
	file.close();


	// Create wxImage from raw data
	const wxImage image(width, height, image_data.data(), true);
	original_image_ = image.Copy();

	if (!image.IsOk())
	{
		wxLogError("Failed to create image from data: %s", file_path_);
		return;
	}

	// Convert wxImage to wxBitmap
	m_bitmap = wxBitmap(image);
	m_bitmapCtrl->SetBitmap(m_bitmap);
	sizer_->Layout();
}


// Exit event handler
void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

// GrayScale event handler
void MainFrame::OnGrayScale(wxCommandEvent& event)
{
	if (is_grayscale_)
	{
		m_tBitmapCtrl->SetBitmap(wxNullBitmap);
		sizer_->Detach(m_tBitmapCtrl);
		sizer_->Layout();
		is_grayscale_ = false;

		return;
	}

	// Set flags for other filters
	if (is_dithered_)
		OnDitherButtonClick(event);

	if (is_auto_leveled_)
		OnAutoLevelButtonClick(event);

	auto image = m_bitmap.ConvertToImage();
	ConvertImageToGrayScale(image);
	// Save the grayscale image for future use
	grayscale_image_ = image;
	m_transformed_bitmap_ = wxBitmap(image);
	m_tBitmapCtrl->SetBitmap(m_transformed_bitmap_);
	sizer_->Add(m_tBitmapCtrl, 1, wxEXPAND | wxRIGHT | wxBOTTOM, 5);
	sizer_->Layout();

	// Toggle flag
	is_grayscale_ = !is_grayscale_;
}


void MainFrame::ConvertImageToGrayScale(wxImage& image)
{
	const int width = image.GetWidth();
	const int height = image.GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const auto red = image.GetRed(x, y);
			const auto green = image.GetGreen(x, y);
			const auto blue = image.GetBlue(x, y);

			const auto gray = (red + green + blue) / 3;

			image.SetRGB(x, y, gray, gray, gray);
		}
	}
}

void MainFrame::OnDitherButtonClick(wxCommandEvent& event)
{
	// Toggle between original bitmap and dithered version
	if (is_dithered_) {
		m_tBitmapCtrl->SetBitmap(wxNullBitmap);
		sizer_->Detach(m_tBitmapCtrl);
		m_bitmapCtrl->SetBitmap(original_image_);
		sizer_->Layout();
		is_dithered_ = false;
		return;
	}

	if (is_grayscale_)
		OnGrayScale(event);
	if (is_auto_leveled_)
		OnAutoLevelButtonClick(event);

	// Perform ordered dithering using a 4x4 Bayer dither matrix
	wxImage image = m_bitmap.ConvertToImage();
	OrderedDither(image, 4);
	m_tBitmapCtrl->SetBitmap(wxBitmap(image));
	m_bitmapCtrl->SetBitmap(wxBitmap(grayscale_image_));
	sizer_->Add(m_tBitmapCtrl, 1, wxEXPAND | wxRIGHT | wxBOTTOM, 5);
	sizer_->Layout();

	is_dithered_ = !is_dithered_;
}

void MainFrame::OrderedDither(wxImage& image, const int matrix_size)
{
	constexpr int matrix[4][4] = {
		{ 0, 8, 2, 10 },
		{ 12, 4, 14, 6 },
		{ 3, 11, 1, 9 },
		{ 15, 7, 13, 5 }
	};

	const int width = image.GetWidth();
	const int height = image.GetHeight();
	ConvertImageToGrayScale(image);

	grayscale_image_ = image;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const int old_pixel = image.GetRed(x, y);
			const int new_pixel = matrix[x % matrix_size][y % matrix_size] * 16;

			// Thresholding to convert to binary image
			constexpr int threshold = 128;
			const uint8_t new_value = old_pixel + new_pixel > threshold ? 255 : 0;

			image.SetRGB(x, y, new_value, new_value, new_value);
		}
	}
}

void MainFrame::OnAutoLevelButtonClick(wxCommandEvent& event)
{
	// Toggle between original bitmap and auto leveled version
	if (is_auto_leveled_)
	{
		m_tBitmapCtrl->SetBitmap(wxNullBitmap);
		sizer_->Detach(m_tBitmapCtrl);
		sizer_->Layout();
		is_auto_leveled_ = false;
		return;
	}

	if (is_grayscale_)
		OnGrayScale(event);

	if (is_dithered_)
		OnDitherButtonClick(event);

	auto image = m_bitmap.ConvertToImage();
	AutoLevel(image);
	m_transformed_bitmap_ = wxBitmap(image);
	m_tBitmapCtrl->SetBitmap(m_transformed_bitmap_);
	sizer_->Add(m_tBitmapCtrl, 1, wxEXPAND | wxRIGHT | wxBOTTOM, 5);
	sizer_->Layout();

	// Toggle flag
	is_auto_leveled_ = !is_auto_leveled_;
}


void MainFrame::AutoLevel(wxImage& image)
{
	const int width = image.GetWidth();
	const int height = image.GetHeight();

	// Find min and max pixel values for each color channel
	int min_pixel_red = 255, min_pixel_green = 255, min_pixel_blue = 255;
	int max_pixel_red = 0, max_pixel_green = 0, max_pixel_blue = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const int pixel_red = image.GetRed(x, y);
			const int pixel_green = image.GetGreen(x, y);
			const int pixel_blue = image.GetBlue(x, y);

			min_pixel_red = std::min(min_pixel_red, pixel_red);
			min_pixel_green = std::min(min_pixel_green, pixel_green);
			min_pixel_blue = std::min(min_pixel_blue, pixel_blue);

			max_pixel_red = std::max(max_pixel_red, pixel_red);
			max_pixel_green = std::max(max_pixel_green, pixel_green);
			max_pixel_blue = std::max(max_pixel_blue, pixel_blue);
		}
	}

	// Apply auto level transformation for each color channel
	const double scale_red = 255.0 / (max_pixel_red - min_pixel_red);
	const double scale_green = 255.0 / (max_pixel_green - min_pixel_green);
	const double scale_blue = 255.0 / (max_pixel_blue - min_pixel_blue);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const int pixel_red = image.GetRed(x, y);
			const int pixel_green = image.GetGreen(x, y);
			const int pixel_blue = image.GetBlue(x, y);

			const int new_pixel_red = static_cast<int>((pixel_red - min_pixel_red) * scale_red);
			const int new_pixel_green = static_cast<int>((pixel_green - min_pixel_green) * scale_green);
			const int new_pixel_blue = static_cast<int>((pixel_blue - min_pixel_blue) * scale_blue);

			image.SetRGB(x, y, new_pixel_red, new_pixel_green, new_pixel_blue);
		}
	}
}

// Huffman Encoding
void MainFrame::OnHuffmanButtonClick(wxCommandEvent& event)
{

	HuffmanEncode();
}

// Huffman Encoding
void MainFrame::HuffmanEncode()
{
	auto image = m_bitmap.ConvertToImage();
	ConvertImageToGrayScale(image);
	grayscale_image_ = image;
	const auto width = image.GetWidth();
	const auto height = image.GetHeight();

	const auto image_data = wxImageToVector(image, width, height);

	const HuffmanEncoder huffman_encoder(image_data);

	const auto huffman_entropy = huffman_encoder.get_entropy();
	const auto huffman_average_length = huffman_encoder.get_average_code_length();

	wxLogMessage("Entropy: %.2f bits/pixel", huffman_entropy);
	wxLogMessage("Average code length: %.2f bits", huffman_average_length);
}

// mirror image
void MainFrame::mirror(std::vector<uint8_t>& image_data, const size_t width, const size_t height)
{

	for (size_t y = 0; y < height; y++)
	{
		for (size_t x = 0; x < width / 2; x++)
		{
			const size_t left_index = (y * width + x) * 3;
			const size_t right_index = (y * width + width - x - 1) * 3;

			std::swap(image_data[left_index], image_data[right_index]);
			std::swap(image_data[left_index + 1], image_data[right_index + 1]);
			std::swap(image_data[left_index + 2], image_data[right_index + 2]);
		}
	}
}

std::vector<uint8_t> MainFrame::wxImageToVector(wxImage& image, const size_t width,const size_t height){
	// Initialize the vector to store pixel data
	std::vector<uint8_t> pixelData(width * height);

	// Get image data
	const unsigned char* imageData = image.GetData();

	// Copy pixel data to the vector
	for (int i = 0; i < width * height; ++i) {
		pixelData[i] = imageData[i];
	}

	return pixelData;
}
