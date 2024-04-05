#pragma once
#include <unordered_map>
#include <queue>
#include <string>

class HuffmanEncoder
{

public:

	[[nodiscard]]
	explicit HuffmanEncoder(const std::vector<uint8_t>& image_data);

	[[nodiscard]]
	double get_average_code_length() const;

	[[nodiscard]]
	double get_entropy() const;

private:
	std::vector<uint8_t> image_data_;
	std::unordered_map<uint8_t, uint64_t> frequency_map_;
	std::unordered_map<uint8_t, std::string> codes_;

	struct Node {
		uint8_t value;
		uint64_t frequency;
		Node* left;
		Node* right;

		Node(const uint8_t value, const uint64_t frequency) : value(value), frequency(frequency), left(nullptr), right(nullptr) {}
	};

	struct Compare
	{
		bool operator()(const Node* left, const Node* right) const
		{
			return left->frequency > right->frequency;
		}
	};

	static Node* build_tree(std::priority_queue<Node*, std::vector<Node*>, Compare> pq);

	void generate_frequency_map();

	[[nodiscard]]
	std::priority_queue<Node*, std::vector<Node*>, Compare> generate_priority_queue() const;

	void generate_codes(const Node* node, const std::string& code);

	[[nodiscard]]
	std::vector<uint8_t> encode() const;
};