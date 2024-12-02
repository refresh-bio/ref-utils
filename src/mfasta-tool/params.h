#pragma once

#include <string>
#include <cinttypes>
#include <vector>

using namespace std;

struct CParams
{
	enum class working_mode_t { none, mrds };

	working_mode_t working_mode = working_mode_t::none;
	vector<string> in_names;
	vector<string> in_prefixes;
	string out_name;
	string out_prefix = "part";
	string out_suffix = "fna";
	bool gzipped_output = false;
	int gzip_level = 4;
	int64_t n = 0;
	int part_digits = 5;
	bool remove_empty_lines = true;
	int no_threads = 4;
	int verbosity = 0;

	// Duplictes
	bool remove_duplicates = false;
	bool rev_comp_as_equivalent = false;
	string out_duplicates;
	bool mark_duplicates_orientation = false;

	// *** Internal params
	const size_t data_source_input_parts_size = 32;
	const size_t soft_limit_size_in_part = 1 << 20;
	const size_t input_queue_max_size = 128;
};
