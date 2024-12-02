#include <iostream>
#include <cstdio>
#include <cstring>
#include <thread>
#include <atomic>

#include "params.h"
#include "data_source.h"
#include "data_storer.h"
#include "data_partitioner.h"
#include "sha256_filter.h"
#include "part_packer.h"

using namespace std;

const string UTIL_VER = "mfasta-tool v. 1.0.4 (2024-12-02)";
const string UTIL_VERSION = "1.0.4";

CParams params;

// *****************************************************************************************
bool parse_mode(int argc, char** argv);
bool parse_args_mrds(int argc, char** argv);
void usage();
bool process_mrds();
vector<string> split(const string& str, char sep);

// ****************************************************************************
vector<string> split(const string& str, char sep)
{
	vector<string> parts;

	string s;

	for (auto c : str)
	{
		if (c == sep)
		{
			parts.emplace_back(s);
			s.clear();
		}
		else
			s.push_back(c);
	}

	if (!s.empty())
		parts.emplace_back(s);

	return parts;
}


// *****************************************************************************************
bool parse_mode(int argc, char** argv)
{
	if (argc < 2)
	{
		return false;
	}

	if (argv[1] == "--version"s)
	{
		params.working_mode = CParams::working_mode_t::info;
		std::cerr << UTIL_VERSION << endl;
		return true;
	}

	if (argv[1] == "mrds"s)
		params.working_mode = CParams::working_mode_t::mrds;

	return params.working_mode != CParams::working_mode_t::none;
}

// *****************************************************************************************
bool parse_args_mrds(int argc, char **argv)
{
	for (int i = 2; i < argc; ++i)
	{
		if ((argv[i] == "-n"s || argv[i] == "--part-size"s) && i + 1 < argc)
		{
			params.n = atoi(argv[i + 1]);
			++i;
		}
		else if ((argv[i] == "-t"s || argv[i] == "--no-threads"s) && i + 1 < argc)
		{
			params.no_threads = atoi(argv[i + 1]);
			if(params.no_threads < 3)
				params.no_threads = 3;
			++i;
		}
		else if ((argv[i] == "-o"s || argv[i] == "--out-name"s) && i + 1 < argc)
		{
			params.out_name = argv[i + 1];
			++i;
		}
		else if (argv[i] == "--out-prefix"s && i + 1 < argc)
		{
			params.out_prefix = argv[i + 1];
			++i;
		}
		else if (argv[i] == "--out-suffix"s && i + 1 < argc)
		{
			params.out_suffix = argv[i + 1];
			++i;
		}
		else if (argv[i] == "--gzipped-output"s)
		{
			params.gzipped_output = true;
		}
		else if (argv[i] == "--gzip-level"s && i + 1 < argc)
		{
			params.gzip_level = atoi(argv[i + 1]);
			++i;
		}
		else if (argv[i] == "--part-digits"s && i + 1 < argc)
		{
			params.part_digits = atoi(argv[i + 1]);
			++i;
		}
		else if (argv[i] == "--verbosity"s && i + 1 < argc)
		{
			params.verbosity = atoi(argv[i + 1]);
			++i;
		}
/*		else if (argv[i] == "--remove-empty-lines"s)
		{
			params.remove_empty_lines = true;
		}*/
		else if (argv[i] == "--remove-duplicates"s)
		{
			params.remove_duplicates = true;
		}
		else if (argv[i] == "--rev-comp-as-equivalent"s)
		{
			params.rev_comp_as_equivalent = true;
		}
		else if (argv[i] == "--mark-duplicates-orientation"s)
		{
			params.mark_duplicates_orientation = true;
		}
		else if (argv[i] == "--out-duplicates"s && i + 1 < argc)
		{
			params.out_duplicates = argv[i + 1];
			++i;
		}
		else if ((argv[i] == "-i"s || argv[i] == "--in-names"s) && i + 1 < argc)
		{
			string fn_list = argv[i + 1];
			++i;
			params.in_names = split(fn_list, ',');
		}
		else if (argv[i] == "--in-prefixes"s && i + 1 < argc)
		{
			string pref_list = argv[i + 1];
			++i;
			params.in_prefixes = split(pref_list, ',');
		}
		else
		{
			std::cerr << "Unknown option: " << argv[i] << endl;
			return false;
		}
	}

	if (params.in_names.empty())
	{
		std::cerr << "Input file name(s) must be provided\n";
		return false;
	}

	if (params.out_name.empty() && params.n == 0)
	{
		std::cerr << "If you want to split the input you mut provide --part-size" << endl;
		return 0;
	}

	if (params.in_prefixes.empty())
		params.in_prefixes.resize(params.in_names.size());
	else if (params.in_prefixes.size() != params.in_names.size())
	{
		std::cerr << "No. of input prefixes (if provided) must be the same as no. of input names" << endl;
		return false;
	}

	return true;
}

// *****************************************************************************************
void usage()
{
	std::cerr << UTIL_VER << endl;
	std::cerr << "Preprocess multi-FASTA files\n";
	std::cerr << "Usage:\n";
	std::cerr << "mfasta-tool <mode> [options]\n";
	std::cerr << "Modes:\n";
	std::cerr << "   mrds - merges a few input files with optional removal of duplicates and splitting into pieces of give size\n";
}

// *****************************************************************************************
void usage_mrds()
{
	std::cerr << UTIL_VER << endl;
	std::cerr << "Preprocess multi-FASTA files\n";
	std::cerr << "Usage:\n";
	std::cerr << "mfasta-tool mrds [options]\n";
	std::cerr << "Options:\n";
	std::cerr << "   -n | --part-size <int>        - no. of sequences in a single output file; 0 - no splitting (default: " << params.n << ")\n";
	std::cerr << "   -o | --out-name <string>      - output name when no splitting is made (default: stdout)\n";
	std::cerr << "   -i | --in-names <string>      - comma-separated list of input file names\n";
	std::cerr << "   --in-prefixes <string>        - comma-separated list of prefixes for input file names (optional)\n";
	std::cerr << "   -t | --no-threads <int>       - no. of threads (default: " << params.no_threads << ")\n";
	std::cerr << "   --out-prefix <string>         - prefix of output file names (default: " << params.out_prefix << ")\n";
	std::cerr << "   --out-suffix <string>         - suffix of output file names (default: " << params.out_suffix << ")\n";
	std::cerr << "   --part-digits <int>           - no. of digits in part_id (default: " << params.part_digits << ")\n";
	std::cerr << "   --gzipped-output              - gzip ouptut files (default: false)\n";
	std::cerr << "   --gzip-level <int>            - compression level for output gzips (default: " << params.gzip_level << ")\n";
	std::cerr << "   --verbosity <int>             - verbosity level (default: " << params.verbosity << ")\n";
//	std::cerr << "   --remove-empty-lines          - remove empty lines\n";
	std::cerr << "   --remove-duplicates           - remove duplicated sequences (same SHA256 checksum) (default: false)\n";
	std::cerr << "   --rev-comp-as-equivalent      - when removing duplicates treat rev. comp. as equivalent (default: false)\n";
	std::cerr << "   --out-duplicates <string>     - name of files with duplicates list (default: stdout)\n";
	std::cerr << "   --mark-duplicates-orientation - mark duplicates orientation ('+' for direct, '-' for rev.comp) (default: false)\n";
	std::cerr << "Example: mfasta-tool mrds -n 1000 -i bacteria.fna\n";
}

// **************************************************
bool process_mrds()
{
	uint32_t n_hashing_threads = 1;
	uint32_t n_packing_threads = 1;
	uint32_t n_min_threads = params.remove_duplicates ? 6 : 4;

	uint32_t n_threads = std::max<uint32_t>(n_min_threads, params.no_threads);
	atomic<bool> is_ok = true;


	if (params.remove_duplicates && params.gzipped_output)
	{
		uint32_t n = n_threads - 4;
		
		if (params.gzip_level <= 4)
		{
			n_hashing_threads = std::max<uint32_t>(1, n / 2);
			n_packing_threads = std::max<uint32_t>(1, n - n_hashing_threads);
		}
		else if (params.gzip_level <= 6)
		{
			n_hashing_threads = std::max<uint32_t>(1, n / 3);
			n_packing_threads = std::max<uint32_t>(1, n - n_hashing_threads);
		}
		else
		{
			n_hashing_threads = std::max<uint32_t>(1, n / 4);
			n_packing_threads = std::max<uint32_t>(1, n - n_hashing_threads);
		}
	}
	else if (params.remove_duplicates)
	{
		n_hashing_threads = n_min_threads - 5;
	}
	else if (params.gzipped_output)
	{
		n_packing_threads = n_min_threads - 3;
	}

	parallel_priority_queue<input_part_t> q_input_parts(params.input_queue_max_size, 1);
	parallel_priority_queue<input_part_t> q_hashed_parts(params.input_queue_max_size, n_hashing_threads);
	parallel_priority_queue<input_part_t> q_filtered_parts(params.input_queue_max_size, 1);
	parallel_priority_queue<input_part_t> q_partitioned_parts(params.input_queue_max_size, 1);
	parallel_priority_queue<packed_part_t> q_packed_parts(params.input_queue_max_size, n_packing_threads);

	size_t no_unique = 0, no_duplicated = 0, no_removed = 0, no_stored = 0;

	thread t_data_source([&is_ok, &q_input_parts] {
		CDataSource data_source(params.in_names, params.in_prefixes, q_input_parts, params.remove_empty_lines, params.data_source_input_parts_size, params.soft_limit_size_in_part, params.verbosity);
		if(!data_source.run())
			is_ok = false;
		});

	vector<thread> vt_sha256_hashers;
	if (params.remove_duplicates)
		for (int i = 0; i < n_hashing_threads; ++i)
			vt_sha256_hashers.emplace_back([&is_ok, &q_input_parts, &q_hashed_parts] {
			CSHA256Hasher part_hasher(q_input_parts, q_hashed_parts, params.rev_comp_as_equivalent);
			if(!part_hasher.run())
				is_ok = false;
				});

	thread t_sha256_filter([&is_ok, &q_hashed_parts, &q_filtered_parts, &no_unique, &no_duplicated, &no_removed] {
		if (params.remove_duplicates)
		{
			CSHA256Filter sha256_filter(params.rev_comp_as_equivalent, params.mark_duplicates_orientation, q_hashed_parts, q_filtered_parts, params.out_duplicates, params.data_source_input_parts_size);
			if(!sha256_filter.run())
				is_ok = false;
			sha256_filter.get_stats(no_unique, no_duplicated, no_removed);
		}
		});

	thread t_data_partitioner([&is_ok, &q_input_parts, &q_filtered_parts, &q_partitioned_parts] {
		CDataPartitioner data_partitioner(params.remove_duplicates ? q_filtered_parts : q_input_parts, q_partitioned_parts, params.n);
		if(!data_partitioner.run())
			is_ok = false;
	});

	vector<thread> vt_data_packers;
	for (int i = 0; i < n_packing_threads; ++i)
		vt_data_packers.emplace_back([&is_ok, &q_partitioned_parts, &q_packed_parts] {
		CPartPacker part_packer(q_partitioned_parts, q_packed_parts, params.gzipped_output, params.gzip_level);
		if(!part_packer.run())
			is_ok = false;
			});

	thread t_data_storer([&is_ok, &q_packed_parts, &no_stored] {
		CDataStorer data_storer(q_packed_parts, params.n, params.out_name, params.out_prefix, params.out_suffix, params.part_digits, params.verbosity);
		if(!data_storer.run())
			is_ok = false;
		data_storer.get_stats(no_stored);
		});

	t_data_source.join();
	t_sha256_filter.join();
	t_data_partitioner.join();
	for (auto& t : vt_sha256_hashers)
		t.join();
	for (auto& t : vt_data_packers)
		t.join();
	t_data_storer.join();

	if (params.verbosity > 0)
	{
		std::cerr << "*** Stats" << endl;
		std::cerr << "No. input sequences: " << (params.remove_duplicates ? (no_unique + no_duplicated + no_removed) : no_stored) << endl;
		if (params.remove_duplicates)
		{
			std::cerr << "   unique          : " << no_unique << endl;
			std::cerr << "   duplicated      : " << no_duplicated << endl;
			std::cerr << "   removed         : " << no_removed << endl;
			std::cerr << "   preserved       : " << no_stored << endl;
		}

		if (params.out_name.empty())
			std::cerr << "No. parts          : " << (no_stored + params.n - 1) / params.n << endl;
	}

	return is_ok;
}

// *****************************************************************************************
int main(int argc, char** argv)
{
	if (!parse_mode(argc, argv))
	{
		usage();
		return 1;
	}

	switch (params.working_mode)
	{
	case CParams::working_mode_t::mrds:
		if (!parse_args_mrds(argc, argv))
		{
			usage_mrds();
			return 1;
		}
		break;
	}

	switch (params.working_mode)
	{
	case CParams::working_mode_t::mrds:
		if (!process_mrds())
			return 1;
		break;
	case CParams::working_mode_t::info:
		return 0;
	default:
		return 1;
	}

	return 0;
}
