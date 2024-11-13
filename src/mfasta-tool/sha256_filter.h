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
#include "utils.h"
#include "sha256.h"

#include <refresh/parallel_queues/lib/parallel-queues.h>

using namespace std;
using namespace refresh;

using sha256_t = refresh::SHA256::sha256_t;

namespace std
{
	template<>
	struct std::hash<sha256_t>
	{
		auto operator()(const sha256_t& x) const -> size_t
		{
			size_t h = 0;

			for (int i = 0; i < 4; ++i)
			{
				h ^= ((uint64_t)(x[2*i])) << 32;
				h ^= ((uint64_t)(x[2*i] + 1));
			}

			auto mh64 = [](uint64_t h) ->uint64_t
				{
					h ^= h >> 33;
					h *= 0xff51afd7ed558ccdL;
					h ^= h >> 33;
					h *= 0xc4ceb9fe1a85ec53L;
					h ^= h >> 33;

					return h;
				};

			return (size_t) mh64(h);
		}
	};
}

class CSHA256Hasher
{
	parallel_priority_queue<input_part_t>& q_input_parts;
	parallel_priority_queue<input_part_t>& q_hashed_parts;
	bool both_dirs;

	char bases_mapping[256];

	refresh::SHA256 hasher_fwd, hasher_rc;

	void upper_case(string& s)
	{
		for (auto& c : s)
			c &= ~((char)32);
	}

	void add_hash(input_item_t &item)
	{
		string tmp;

		hasher_fwd.reset();

		for (auto& s : item.lines)
		{
			upper_case(s);

			if (s != "\n"s)
				hasher_fwd.update(s);
		}

		hasher_fwd.finalize();
		item.hash = hasher_fwd.get_hash();
		item.hash_orientation_fwd = true;

		if (both_dirs)
		{
			hasher_rc.reset();

			for (auto p = item.lines.rbegin(); p != item.lines.rend(); ++p)
			{
				size_t len = p->size();
				const char* d = p->data();

				tmp.resize(len);
				for (size_t i = 0; i < len; ++i)
					tmp[len - 1 - i] = bases_mapping[d[i]];

				if (tmp != "\n"s)
					hasher_rc.update(tmp);
			}

			hasher_rc.finalize();

			if (hasher_rc.get_hash() < hasher_fwd.get_hash())
			{
				item.hash = hasher_rc.get_hash();
				item.hash_orientation_fwd = false;
			}
		}
	}

public:
	CSHA256Hasher(parallel_priority_queue<input_part_t>& q_input_parts, parallel_priority_queue<input_part_t>& q_hashed_parts, bool both_dirs) :
		q_input_parts(q_input_parts),
		q_hashed_parts(q_hashed_parts),
		both_dirs(both_dirs)
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

	void run()
	{
		uint64_t priority;
		input_part_t input_part;

		while (q_input_parts.pop(input_part, priority))
		{
			for (auto& item : input_part)
				add_hash(item);

			q_hashed_parts.push(priority, move(input_part));
			input_part.clear();
		}

		q_hashed_parts.mark_completed();
	}
};

class CSHA256Filter
{
	bool rev_comp_as_equivalent;
	bool mark_duplicates_orientation;
	parallel_priority_queue<input_part_t>& q_input_parts;
	parallel_priority_queue<input_part_t>& q_filtered_parts;
	string out_log_fn;
	size_t no_seq_in_part;

	unordered_map<sha256_t, list<pair<bool, string>>> dict;

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

	// Return false if duplicated
	bool add_to_dict(const input_item_t &input_item)
	{
		auto p = dict.find(input_item.hash);

		if (p != dict.end())
		{
			p->second.emplace_back(input_item.hash_orientation_fwd, prepare_id(input_item.id, input_item.prefix));
			return false;
		}

		dict[input_item.hash].emplace_back(input_item.hash_orientation_fwd, prepare_id(input_item.id, input_item.prefix));
		return true;
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
			bool is_first_fwd;
			for (const auto &di : dict_item.second)
			{
				if (is_first)
				{
					ofs << di.second;
					is_first = false;
					is_first_fwd = di.first;
				}
				else
				{
					ofs << " ";
					if (mark_duplicates_orientation)
						ofs << ((is_first_fwd ^ di.first) ? "+" : "-");
					ofs << di.second;
				}
			}

			ofs << endl;
		}
	}

public:
	CSHA256Filter(bool rev_comp_as_equivalent, bool mark_duplicates_orientation,
		parallel_priority_queue<input_part_t>& q_input_parts, parallel_priority_queue<input_part_t>& q_filtered_parts,
		const string &out_log_fn, const size_t no_seq_in_part) :
		rev_comp_as_equivalent(rev_comp_as_equivalent),
		mark_duplicates_orientation(mark_duplicates_orientation),
		q_input_parts(q_input_parts),
		q_filtered_parts(q_filtered_parts),
		out_log_fn(out_log_fn),
		no_seq_in_part(no_seq_in_part)
	{}

	bool run()
	{
		input_part_t input_part;
		input_part_t filtered_part;
		uint64_t priority;

		dict.max_load_factor(1.0);

		while (q_input_parts.pop(input_part, priority))
		{
			for (auto& item : input_part)
				if (!add_to_dict(item))
					item.lines.clear();

			input_part.erase(remove_if(input_part.begin(), input_part.end(), [](const auto& x) {return x.lines.empty(); }), input_part.end());

			q_filtered_parts.push(priority, move(input_part));
		}

		q_filtered_parts.mark_completed();

		if(out_log_fn.empty())
			store_log(cout);
		else
		{
			ofstream ofs(out_log_fn, std::ios::binary);

			if (!ofs)
			{
				std::cerr << "Cannot open duplicated log file: " << out_log_fn << endl;
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