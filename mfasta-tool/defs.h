#pragma once

#include <string>
#include <vector>
#include <cinttypes>

using namespace std;

struct input_item_t
{
	string id;
	vector<string> lines;

	input_item_t(const string& id, const vector<string>& lines) :
		id(id), lines(lines)
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

/*struct partitioned_part_t
{
	size_t priority = 0;
	input_part_t part;

	partitioned_part_t() = default;

	partitioned_part_t(const size_t priority, const input_part_t& part) :
		priority(priority),
		part(part)
	{}
};

*/