#pragma once

#include <string>
#include <vector>
#include <cinttypes>

#include "sha256.h"

using namespace std;

struct input_item_t
{
	string id;
	string prefix;
	refresh::SHA256::sha256_t hash{};
	bool hash_orientation_fwd;
	vector<string> lines;

	input_item_t(const string& id, const string &prefix, const vector<string>& lines) :
		id(id), prefix(prefix), hash{}, hash_orientation_fwd(true), lines(lines)
	{}
};

using input_part_t = vector<input_item_t>;

using memory_block_t = vector<uint8_t>;

struct packed_part_t
{
	size_t no_items;
	memory_block_t memory_block;

	packed_part_t() : no_items(0)
	{}

	packed_part_t(size_t no_items, const memory_block_t& memory_block) :
		no_items(no_items),
		memory_block(memory_block)
	{}

	packed_part_t(size_t no_items, memory_block_t&& memory_block) :
		no_items(no_items),
		memory_block(move(memory_block))
	{}

	void clear()
	{
		memory_block.clear();

		if(memory_block.capacity() > 8 << 20)
			memory_block.shrink_to_fit();
	}
};
