#pragma once

#include <unordered_map>
#include <list>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <functional>
#include <cinttypes>

#include "params.h"
#include "md5_wrapper.h"
#include "utils.h"

#include <refresh/parallel_queues/lib/parallel-queues.h>

using namespace std;
using namespace refresh;

using md5_t = vector<uint8_t>;

namespace std
{
	template<>
	struct std::hash<md5_t>
	{
		auto operator()(const md5_t& x) const -> size_t
		{
			uint64_t first = 0;
			uint64_t second = 0;

			for (int i = 0; i < 8; ++i)
				first += ((uint64_t)x[i]) << (8 * i);
			for (int i = 0; i < 8; ++i)
				second += ((uint64_t)x[i+8]) << (8 * i);

			auto mh64 = [](uint64_t h) ->uint64_t
				{
					h ^= h >> 33;
					h *= 0xff51afd7ed558ccdL;
					h ^= h >> 33;
					h *= 0xc4ceb9fe1a85ec53L;
					h ^= h >> 33;

					return h;
				};

			return (size_t) (mh64(first) ^ mh64(second));
		}
	};
}

class CMD5Filter
{
	bool rev_comp_as_equivalent;
	bool mark_duplicates_orientation;
	parallel_queue<input_part_t>& q_input_parts;
	parallel_queue<input_part_t>& q_filtered_parts;
	string out_log_fn;
	size_t no_seq_in_part;

	char bases_mapping[256];

	unordered_map<md5_t, list<pair<bool, string>>> dict;

	size_t no_unique, no_duplicated, no_removed;

	string strip_id(const string& s)
	{
		vector<char> term_symbols = { ' ', '\t', '\n' };
		auto p = find_first_of(s.begin(), s.end(), term_symbols.begin(), term_symbols.end());

		return string(s.begin(), p);
	}

	string prepare_id(const string& id, const string& prefix)
	{
		return build_new_id(strip_id(id), prefix);
	}

	md5_t get_md5(const vector<string>& input_data)
	{
		CMD5 md5;

		for (const auto& s : input_data)
			if(s != "\n"s)
				md5.Update((uint8_t*) s.data(), s.size());

		return md5.Get();
	}

	pair<md5_t, md5_t> get_md5s(const vector<string>& input_data)
	{
		CMD5 md5_fwd, md5_rc;
		string tmp;

		for (const auto& s : input_data)
			if (s != "\n"s)
				md5_fwd.Update((uint8_t*)s.data(), s.size());

		for (auto p = input_data.rbegin(); p != input_data.rend(); ++p)
		{
			size_t len = p->size();
			const char *d = p->data();

			tmp.resize(len);
			for (size_t i = 0; i < len; ++i)
				tmp[len - 1 - i] = bases_mapping[d[i]];

			if (tmp != "\n"s)
				md5_rc.Update((uint8_t*) tmp.data(), len);
		}

		return make_pair(md5_fwd.Get(), md5_rc.Get());
	}

	// Return false if duplicated
	bool add_to_dict(const input_item_t &input_item)
	{
		if (rev_comp_as_equivalent)
		{
			auto item_md5s = get_md5s(input_item.lines);

			auto p = dict.find(item_md5s.first);

			if (p != dict.end())
			{
				p->second.emplace_back(true, prepare_id(input_item.id, input_item.prefix));
				return false;
			}

			p = dict.find(item_md5s.second);

			if (p != dict.end())
			{
				p->second.emplace_back(false, prepare_id(input_item.id, input_item.prefix));
				return false;
			}

			dict[item_md5s.first].emplace_back(true, prepare_id(input_item.id, input_item.prefix));

			return true;
		}
		else
		{
			auto item_md5 = get_md5(input_item.lines);

			auto p = dict.find(item_md5);

			if (p != dict.end())
			{
				p->second.emplace_back(true, prepare_id(input_item.id, input_item.prefix));
				return false;
			}

			dict[item_md5].emplace_back(true, prepare_id(input_item.id, input_item.prefix));

			return true;
		}	
	}

	void store_log(ostream &ofs)
	{
		no_unique = no_duplicated = no_removed = 0;

		for (auto& dict_item : dict)
		{
			if (dict_item.second.size() == 1)
			{
				++no_unique;
				continue;
			}

//			ofs << dict_item.second.front().second << " ";

			++no_duplicated;
			no_removed += dict_item.second.size() - 1;

			bool is_first = true;
			for (const auto &di : dict_item.second)
			{
				if (is_first)
				{
					ofs << di.second;
					is_first = false;
				}
				else
				{
					ofs << " ";
					if (mark_duplicates_orientation)
						ofs << (di.first ? "+" : "-");
					ofs << di.second;
				}
			}

			ofs << endl;
		}
	}

public:
	CMD5Filter(bool rev_comp_as_equivalent, bool mark_duplicates_orientation,
		parallel_queue<input_part_t>& q_input_parts, parallel_queue<input_part_t>& q_filtered_parts, 
		const string &out_log_fn, const size_t no_seq_in_part) :
		rev_comp_as_equivalent(rev_comp_as_equivalent),
		mark_duplicates_orientation(mark_duplicates_orientation),
		q_input_parts(q_input_parts),
		q_filtered_parts(q_filtered_parts),
		out_log_fn(out_log_fn),
		no_seq_in_part(no_seq_in_part)
	{
		for (int i = 0; i < 256; ++i)
			bases_mapping[i] = (char)i;

		bases_mapping['A'] = 'T';
		bases_mapping['C'] = 'G';
		bases_mapping['G'] = 'C';
		bases_mapping['T'] = 'A';

		bases_mapping['a'] = 't';
		bases_mapping['c'] = 'g';
		bases_mapping['g'] = 'c';
		bases_mapping['t'] = 'a';
	}

	bool run()
	{
		input_part_t input_part;
		input_part_t filtered_part;

		while (q_input_parts.pop(input_part))
		{
			for(const auto &item : input_part)
				if (add_to_dict(item))
					filtered_part.emplace_back(item);

			if (!filtered_part.empty())
				q_filtered_parts.push(move(filtered_part));
			filtered_part.clear();
		}

		q_filtered_parts.mark_completed();

		if(out_log_fn.empty())
			store_log(cout);
		else
		{
			ofstream ofs(out_log_fn, std::ios::binary);

			if (!ofs)
			{
				cerr << "Cannot open duplicated log file: " << out_log_fn << endl;
				return false;
			}

			store_log(ofs);
		}

		return true;
	}

	void get_stats(size_t& _no_unique, size_t& _no_duplicated, size_t& _no_removed)
	{
		_no_unique = no_unique;
		_no_duplicated = no_duplicated;
		_no_removed = no_removed;
	}
};