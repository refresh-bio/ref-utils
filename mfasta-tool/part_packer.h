#pragma once

#include "defs.h"
#include "data_source.h"

#include <refresh/compression/lib/gz_wrapper.h>
#include <refresh/parallel_queues/lib/parallel-queues.h>

using namespace refresh;

class CPartPacker
{
	parallel_priority_queue<input_part_t>& q_partitioned_parts;
	parallel_priority_queue<packed_part_t>& q_packed_parts;
	bool gzipped_output;
	int gzip_level;

	packed_part_t packed_part;
	memory_block_t buffer;

	gz_in_memory gim;

	void do_pack(input_part_t& input_part)
	{
		size_t raw_size = 0;

		for (const auto& item : input_part)
		{
			raw_size += item.id.size() + 1;
			raw_size += item.prefix.size();
			for (const auto& line : item.lines)
				raw_size += line.size() + 1;
		}

		buffer.clear();
		buffer.reserve(raw_size);

		for (const auto& item : input_part)
		{
//			buffer.insert(buffer.end(), item.id.begin(), item.id.end());
			buffer.push_back(item.id.front());
			buffer.insert(buffer.end(), item.prefix.begin(), item.prefix.end());
			buffer.insert(buffer.end(), item.id.begin() + 1, item.id.end());
			buffer.emplace_back('\n');

			for (const auto& line : item.lines)
			{
				buffer.insert(buffer.end(), line.begin(), line.end());
				if(line.size() > 1 || (line.size() == 1 && line.front() != '\n'))
					buffer.emplace_back('\n');
			}
		}

		if (gzipped_output)
		{
			packed_part.memory_block.resize(raw_size + gim.get_overhead(raw_size));
			auto packed_size = gim.compress(buffer.data(), buffer.size(), packed_part.memory_block.data(), packed_part.memory_block.size());
			packed_part.memory_block.resize(packed_size);
		}
		else
			packed_part.memory_block.swap(buffer);

		packed_part.no_items = input_part.size();

		input_part.clear();

		if (buffer.capacity() > 8 << 20)
		{
			buffer.clear();
			buffer.shrink_to_fit();
		}
	}

public:
	CPartPacker(parallel_priority_queue<input_part_t>& q_partitioned_parts, parallel_priority_queue<packed_part_t>& q_packed_parts,
		bool gzipped_output, int gzip_level) :
		q_partitioned_parts(q_partitioned_parts),
		q_packed_parts(q_packed_parts),
		gzipped_output(gzipped_output),
		gzip_level(gzip_level),
		gim(gzip_level)
	{}

	void run()
	{
		input_part_t input_part;
		uint64_t priority;

		while (q_partitioned_parts.pop(input_part, priority))
		{
			do_pack(input_part);

			q_packed_parts.push(priority, move(packed_part));
			packed_part.clear();
		}

		q_packed_parts.mark_completed();
	}
};
