#include "HuffmanEncoder.h"
#include <queue>
#include <ranges>
#include <string>

HuffmanEncoder::HuffmanEncoder(const std::vector<uint8_t>& image_data) :image_data_(image_data)
{
	// Count the frequency of each byte in the image data
	generate_frequency_map();

	// Create a priority queue of nodes
	const auto pq = generate_priority_queue();

	// Build the Huffman tree
	const auto root = build_tree(pq);

    // Generate the Huffman codes for each symbol
    generate_codes(root, "");

}

void HuffmanEncoder::generate_frequency_map()
{
	// Loop through the image data and count the frequency of each byte
	for (uint8_t color : image_data_)
	{
		frequency_map_[color]++;
	}
}

std::priority_queue<HuffmanEncoder::Node*, std::vector<HuffmanEncoder::Node*>, HuffmanEncoder::Compare> HuffmanEncoder::generate_priority_queue() const
{
	std::priority_queue<Node*, std::vector<Node*>, Compare> pq;

	// Loop through the frequency map and create a node for each byte
	for (const auto& [fst, snd] : frequency_map_)
	{
		auto node = new Node(fst, snd);
		pq.push(node);
	}

	return pq;
}

HuffmanEncoder::Node* HuffmanEncoder::build_tree(std::priority_queue<Node*, std::vector<Node*>, Compare> pq)
{
	// Keep looping until there is only one node left in the priority queue
	while (pq.size() > 1)
	{
		// Get the two nodes with the lowest frequencies
		Node* left = pq.top();
		pq.pop();
		Node* right = pq.top();
		pq.pop();

		// Create a new node with the two nodes as children
		auto parent = new Node('\0', left->frequency + right->frequency);
		parent->left = left;
		parent->right = right;

		// Add the parent node back to the priority queue
		pq.push(parent);
	}

	return pq.top();
}


void HuffmanEncoder::generate_codes(const Node* node, const std::string& code)
{
    if (node->left == nullptr && node->right == nullptr)
    {
        // Leaf node, store the Huffman code for this symbol
        codes_[node->value] = code;
    }
    else
    {
        // Recursive calls for left and right children
        generate_codes(node->left, code + '0');
        generate_codes(node->right, code + '1');
    }
}

double HuffmanEncoder::get_average_code_length() const
{
    double total_length = 0.0;
    size_t total_symbols = 0;

    // Loop through all symbols and calculate the total length
    for (const auto& [symbol, code] : codes_)
    {
        total_length += code.length() * frequency_map_.at(symbol);
        total_symbols += frequency_map_.at(symbol);
    }

    // Calculate the average code length
    return total_length / total_symbols;
}

double HuffmanEncoder::get_entropy() const
{
    double entropy = 0.0;
    size_t total_symbols = 0;

    // Loop through all symbols and calculate the entropy

    for (const auto& frequency : frequency_map_ | std::views::values)
    {
        const double probability = static_cast<double>(frequency) / static_cast<double>(image_data_.size());
        entropy += probability * std::log2(probability);
        total_symbols += frequency;
    }

    entropy *= -1.0; // Apply the negative sign to get the final result

    return entropy;
}

std::vector<uint8_t> HuffmanEncoder::encode() const
{
    std::vector<uint8_t> encoded_data;

    // Encode each byte in the input data
    for (uint8_t color : image_data_)
    {
        // Append the Huffman code for this byte to the encoded data
        std::string code;
        code = codes_.at(color);
        for (const char bit : code)
        {
            if (bit == '0')
                encoded_data.push_back(0);
            else
                encoded_data.push_back(1);
        }
    }

    return encoded_data;
}