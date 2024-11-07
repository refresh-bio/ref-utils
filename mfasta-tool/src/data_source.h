#pragma once

#include "defs.h"
#include "params.h"

#include <refresh/parallel_queues/lib/parallel-queues.h>
#include <refresh/compression/lib/file_wrapper.h>

using namespace refresh;

class CDataSource
{
	vector<string> input_names;
	vector<string> input_prefixes;
	parallel_queue<input_part_t> &q_input_parts;
	size_t no_seq_in_part;
	size_t soft_limit_size_in_part;
	bool remove_empty_lines;

	input_part_t input_buffer;

	void upper_case(string& s)
	{
		for (auto& c : s)
			c &= ~((char)32);
	}

	bool load_file(const string& fn, const string &prefix)
	{
		stream_in_file msgz(fn);

		cerr << "Processing " << fn << endl;

		if (!msgz.is_open())
		{
			cerr << "Error: cannot open " << fn << endl;
			return false;
		}

		stream_decompression sdf(&msgz);
		string line;
		size_t seq_len_in_part = 0;
		size_t no_seqs = 0;

		while (!sdf.eof())
		{
			if (sdf.getline(line) == 0)
			{
				if (line.size() < 60)
					int aa = 1;
				if (line.empty())
				{
					if (!remove_empty_lines)
						if (!input_buffer.empty())
							input_buffer.back().lines.push_back("\n");

					continue;
				}

				if (line.front() == '>')
				{
					++no_seqs;
					if (input_buffer.size() == no_seq_in_part || seq_len_in_part >= soft_limit_size_in_part)
					{
						q_input_parts.push(move(input_buffer));
						input_buffer.clear();
						seq_len_in_part = 0;
					}

					input_buffer.emplace_back(line, prefix, vector<string>());
					continue;
				}

				upper_case(line);
				input_buffer.back().lines.emplace_back(line);
				seq_len_in_part += line.size();
			}
		}

		if (!input_buffer.empty())
			q_input_parts.push(move(input_buffer));

		cerr << "Processed " << fn << " - " << no_seqs << " sequences " << endl;

		return true;
	}

public:
	CDataSource(const vector<string>& input_names, const vector<string>& input_prefixes, parallel_queue<input_part_t> &q_input_parts, bool remove_empty_lines, const size_t no_seq_in_part, const size_t soft_limit_size_in_part) :
		input_names(input_names),
		input_prefixes(input_prefixes),
		q_input_parts(q_input_parts),
		remove_empty_lines(remove_empty_lines),
		no_seq_in_part(no_seq_in_part),
		soft_limit_size_in_part(soft_limit_size_in_part)
	{
	}

	bool run()
	{
		for (size_t i = 0; i < input_names.size(); ++i)
			if (!load_file(input_names[i], input_prefixes[i]))
			{
				q_input_parts.mark_completed();
				return false;
			}

		q_input_parts.mark_completed();

		return true;
	}
};