#pragma once

#include "params.h"
#include "data_source.h"
#include <memory>
#include <list>
#include <thread>

class CDataStorer
{
	parallel_priority_queue<packed_part_t>& q_packed_parts;
	size_t n_in_part;
	string out_name;
	string out_prefix;
	string out_suffix;
	int part_digits;
	int verbosity;

	int part_id = 0;
	size_t no_stored = 0;

	string part_fn()
	{
		string id(part_digits, '0');
		id += to_string(part_id);
		id = id.substr(id.length() - part_digits, part_digits);

		return out_prefix + "_" + id + "." + out_suffix;
	}

	bool open_file(FILE* &out, const string& fn)
	{
		if (out)
			fclose(out);

		out = fopen(fn.c_str(), "wb");

		if (!out)
		{
			cerr << "Cannot open file: " << fn << endl;
			return false;
		}

		setvbuf(out, nullptr, _IOFBF, 16 << 20);
		
		return true;
	}

public:
	CDataStorer(parallel_priority_queue<packed_part_t>& q_packed_parts, size_t _n_in_part,
		string out_name, string out_prefix, string out_suffix, int part_digits, int verbosity) :
		q_packed_parts(q_packed_parts),
		n_in_part(_n_in_part),
		out_name(out_name),
		out_prefix(out_prefix),
		out_suffix(out_suffix),
		part_digits(part_digits),
		verbosity(verbosity)
	{
		if (!out_name.empty())
			n_in_part = ~(size_t)0;
	}

	bool run()
	{
		packed_part_t input_part;
		size_t curr_part_size = 0;
		FILE* out = nullptr;

		if (!open_file(out, out_name.empty() ? part_fn() : out_name))
			return false;

		if (verbosity > 0)
			cerr << "Part: " << part_id << "\r";

		while (q_packed_parts.pop(input_part))
		{
			fwrite(input_part.memory_block.data(), 1, input_part.memory_block.size(), out);
			curr_part_size += input_part.no_items;

			no_stored += input_part.no_items;

			if (curr_part_size >= n_in_part)
			{
				fclose(out);
				++part_id;

				if (!open_file(out, part_fn()))
					return false;

				if (verbosity > 0)
					cerr << "Part: " << part_id << "\r";

				curr_part_size = 0;
			}
		}

		if (out)
			fclose(out);
		
		return true;
	}

	void get_stats(size_t& _no_stored)
	{
		_no_stored = no_stored;
	}
};