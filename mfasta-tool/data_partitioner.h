#pragma once

#include "defs.h"
#include "params.h"
#include "data_source.h"

class CDataPartitioner
{
	parallel_priority_queue<input_part_t>& q_input_parts;
	parallel_priority_queue<input_part_t>& q_partitioned_parts;
	size_t n_in_part;

public:
	CDataPartitioner(parallel_priority_queue<input_part_t>& q_input_parts, parallel_priority_queue<input_part_t>& q_partitioned_parts, size_t n_in_part) :
		q_input_parts(q_input_parts),
		q_partitioned_parts(q_partitioned_parts),
		n_in_part(n_in_part)
	{}

	void run()
	{
		input_part_t input_part;
		input_part_t partitioned_part;
		size_t curr_part_size = 0;
		size_t priority = 0;

		while (q_input_parts.pop(input_part))
		{
			for (auto& item : input_part)
			{
				partitioned_part.emplace_back(move(item));
				if (++curr_part_size == n_in_part)
				{
					q_partitioned_parts.push(priority, move(partitioned_part));

					partitioned_part.clear();
					curr_part_size = 0;
					++priority;
				}
			}

			if (!partitioned_part.empty())
			{
				q_partitioned_parts.push(priority, move(partitioned_part));

				partitioned_part.clear();
				curr_part_size = 0;
				++priority;
			}
		}

		if(!partitioned_part.empty())
			q_partitioned_parts.push(priority, move(partitioned_part));

		q_partitioned_parts.mark_completed();
	}
};