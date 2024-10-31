#include <iostream>
#include <cstdio>
#include <cstring>
#include <thread>

#include "params.h"
#include "data_source.h"
#include "data_storer.h"
#include "data_partitioner.h"
#include "md5_filter.h"
#include "part_packer.h"

using namespace std;

const string UTIL_VER = "mfasta-tool v. 1.0.2 (2024-10-30)";
const string UTIL_VERSION = "1.0.2";

CParams params;

// *****************************************************************************************
bool parse_mode(int argc, char** argv);
bool parse_args_mrds(int argc, char** argv);
void usage();
void process_mrds();
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
		cerr << UTIL_VERSION << endl;
		return true;
	}

	if (argv[1] == "mrds"s)
		params.working_mode = CParams::working_mode_t::mrds;

	return params.working_mode != CParams::working_mode_t::none;
}

// *****************************************************************************************
bool parse_args_mrds(int argc, char **argv)
{
	for (int i = 2; i < argc - 1; ++i)
	{
		if ((argv[i] == "-n"s || argv[i] == "--part-size"s) && i + 1 < argc)
		{
			params.n = atoi(argv[i + 1]);
			++i;
		}
		if ((argv[i] == "-t"s || argv[i] == "--no-threads"s) && i + 1 < argc)
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
		else if (argv[i] == "--remove-empty-lines"s)
		{
			params.remove_empty_lines = true;
		}
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
			cerr << "Unknown option: " << argv[i] << endl;
			return false;
		}
	}

	if (params.in_names.empty())
	{
		cerr << "Input file name(s) must be provided\n";
		return false;
	}

	if (params.out_name.empty() && params.n == 0)
	{
		cerr << "If you want to split the input you mut provide --part-size" << endl;
		return 0;
	}

	if (params.in_prefixes.empty())
		params.in_prefixes.resize(params.in_names.size());
	else if (params.in_prefixes.size() != params.in_names.size())
	{
		cerr << "No. of input prefixes (if provided) must be the same as no. of input names" << endl;
		return false;
	}

	return true;
}

// *****************************************************************************************
void usage()
{
	cerr << UTIL_VER << endl;
	cerr << "Preprocess multi-FASTA files\n";
	cerr << "Usage:\n";
	cerr << "mfasta-tool <mode> [options]\n";
	cerr << "Modes:\n";
	cerr << "   mrds - merges a few input files with optional removal of duplicates and splitting into pieces of give size\n";
}

// *****************************************************************************************
void usage_mrds()
{
	cerr << UTIL_VER << endl;
	cerr << "Preprocess multi-FASTA files\n";
	cerr << "Usage:\n";
	cerr << "mfasta-tool mrds [options]\n";
	cerr << "Options:\n";
	cerr << "   -n | --part-size <int>        - no. of sequences in a single output file; 0 - no splitting (default: " << params.n << ")\n";
	cerr << "   -o | --out-name <string>      - output name when no splitting is made (default: stdout)\n";
	cerr << "   -i | --in-names <string>      - comma-separated list of input file names\n";
	cerr << "   -t | --no-threads <int>       - no. of threads (default: " << params.no_threads << ")\n";
	cerr << "   --out-prefix <string>         - prefix of output file names (default: " << params.out_prefix << ")\n";
	cerr << "   --out-suffix <string>         - suffix of output file names (default: " << params.out_suffix << ")\n";
	cerr << "   --part-digits <int>           - no. of digits in part_id (default: " << params.part_digits << ")\n";
	cerr << "   --gzipped-output              - gzip ouptut files (default: false)\n";
	cerr << "   --gzip-level <int>            - compression level for output gzips (default: " << params.gzip_level << ")\n";
	cerr << "   --verbosity <int>             - verbosity level (default: " << params.verbosity << ")\n";
	cerr << "   --remove-empty-lines          - remove empty lines\n";
	cerr << "   --remove-duplicates           - remove duplicated sequences (same MD5 checksum) (default: false)\n";
	cerr << "   --rev-comp-as-equivalent      - when removing duplicates treat rev. comp. as equivalent (default: false)\n";
	cerr << "   --out-duplicates <string>     - name of files with duplicates list (default: stdout)\n";
	cerr << "   --mark-duplicates-orientation - mark duplicates orientation ('+' for direct, '-' for rev.comp) (default: false)\n";
	cerr << "Example: mfasta-tool -n 1000 bacteria.fna\n";
}

// **************************************************
void process_mrds()
{
	int no_packing_threads = params.no_threads;

	parallel_queue<input_part_t> q_input_parts(params.input_queue_max_size, 1);
	parallel_queue<input_part_t> q_filtered_parts(params.input_queue_max_size, 1);
	parallel_priority_queue<input_part_t> q_partitioned_parts(params.input_queue_max_size, 1);
	parallel_priority_queue<packed_part_t> q_packed_parts(params.input_queue_max_size, no_packing_threads);

	size_t no_unique, no_duplicated, no_removed, no_stored;

	thread t_data_source([&q_input_parts] {
		CDataSource data_source(params.in_names, params.in_prefixes, q_input_parts, params.remove_empty_lines, params.data_source_input_parts_size, params.soft_limit_size_in_part);
		data_source.run();
		});

	thread t_md5_filter([&q_input_parts, &q_filtered_parts, &no_unique, &no_duplicated, &no_removed] {
		if (params.remove_duplicates)
		{
			CMD5Filter md5_filter(params.rev_comp_as_equivalent, params.mark_duplicates_orientation, q_input_parts, q_filtered_parts, params.out_duplicates, params.data_source_input_parts_size);
			md5_filter.run();
			md5_filter.get_stats(no_unique, no_duplicated, no_removed);
		}
		});

	thread t_data_partitioner([&q_input_parts, &q_filtered_parts, &q_partitioned_parts] {
		CDataPartitioner data_partitioner(params.remove_duplicates ? q_filtered_parts : q_input_parts, q_partitioned_parts, params.n);
		data_partitioner.run();
	});

	vector<thread> vt_data_packers;
	for (int i = 0; i < no_packing_threads; ++i)
		vt_data_packers.emplace_back([&q_partitioned_parts, &q_packed_parts] {
		CPartPacker part_packer(q_partitioned_parts, q_packed_parts, params.gzipped_output, params.gzip_level);
		part_packer.run();
			});

	thread t_data_storer([&q_packed_parts, &no_stored] {
		CDataStorer data_storer(q_packed_parts, params.n, params.out_name, params.out_prefix, params.out_suffix, params.part_digits, params.verbosity);
		data_storer.run();
		data_storer.get_stats(no_stored);
		});

	t_data_source.join();
	t_md5_filter.join();
	t_data_partitioner.join();
	for (auto& t : vt_data_packers)
		t.join();
	t_data_storer.join();

	cerr << "*** Stats" << endl;
	cerr << "No. input sequences: " << no_unique + no_duplicated + no_removed << endl;
	if (params.remove_duplicates)
	{
		cerr << "   unique          : " << no_unique << endl;
		cerr << "   duplicated      : " << no_duplicated << endl;
		cerr << "   removed         : " << no_removed << endl;
		cerr << "   preserved       : " << no_stored << endl;
	}

	if(params.out_name.empty())
		cerr << "No. parts          : " << (no_stored + params.n - 1) / params.n;
}

// *****************************************************************************************
int main(int argc, char** argv)
{
	if (!parse_mode(argc, argv))
	{
		usage();
		return 0;
	}

	switch (params.working_mode)
	{
	case CParams::working_mode_t::mrds:
		if (!parse_args_mrds(argc, argv))
		{
			usage_mrds();
			return 0;
		}
		break;
	}

	switch (params.working_mode)
	{
	case CParams::working_mode_t::mrds:
		process_mrds();
		break;
	}

	return 0;
}
