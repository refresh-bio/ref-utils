#include <iostream>
#include <string>
#include <cinttypes>
#include <cstdio>
#include <cstring>

#include "../libs/refresh/gz_wrapper.h"
#include "../libs/refresh/file_wrapper.h"

using namespace std;
using namespace refresh;

const string UTIL_VER = "multi-fasta-split v. 1.0.1 (2024-10-22)";
const string UTIL_VERSION = "1.0.1";

enum class working_mode_t { none, splitting };

string in_name;
string out_prefix = "part";
string out_suffix = "fna";
bool gzipped_output = false;
int gzip_level = 6;
int64_t n = 1000;
int part_digits = 5;
bool remove_empty_lines = false;
int verbosity = 0;
working_mode_t working_mode = working_mode_t::none;

const size_t IN_BUF_SIZE = 64 << 20;
vector<uint8_t> in_buf;
vector<uint8_t> out_buf;
gz_in_memory giz;

// **************************************************
bool parse_args(int argc, char** argv);
void usage();
void store_buffer(FILE* f_out);
void process();
string part_fn(int part_id, const int no_digits);

// **************************************************
bool parse_args(int argc, char** argv)
{
	if (argc < 2)
		return false;

	if (argv[1] == "--version"s)
	{
		cerr << UTIL_VERSION << endl;
		return true;
	}

	working_mode = working_mode_t::splitting;	in_name = argv[argc - 1];

	for (int i = 1; i < argc - 1; ++i)
	{
		if (argv[i] == "-n"s && i + 1 < argc - 1)
		{
			n = atoi(argv[i + 1]);
			++i;
		}
		else if (argv[i] == "--out-prefix"s && i + 1 < argc - 1)
		{
			out_prefix = argv[i + 1];
			++i;
		}
		else if (argv[i] == "--out-suffix"s && i + 1 < argc - 1)
		{
			out_suffix = argv[i + 1];
			++i;
		}
		else if (argv[i] == "--gzipped-output"s)
		{
			gzipped_output = true;
		}
		else if (argv[i] == "--gzip-level"s && i + 1 < argc - 1)
		{
			gzip_level = atoi(argv[i + 1]);
			++i;
		}
		else if (argv[i] == "--part-digits"s && i + 1 < argc - 1)
		{
			part_digits = atoi(argv[i + 1]);
			++i;
		}
		else if (argv[i] == "--verbosity"s && i + 1 < argc - 1)
		{
			verbosity = atoi(argv[i + 1]);
			++i;
		}
		else if (argv[i] == "--remove-empty-lines"s)
		{
			remove_empty_lines = true;
		}
		else
		{
			cerr << "Unknown option: " << argv[i] << endl;
			return false;
		}
	}

	return true;
}

// **************************************************
void usage()
{
	cerr << UTIL_VER << endl;
	cerr << "Splits multi-FASTA file names into pieces containing given no. of sequences\n";
	cerr << "Usage:\n";
	cerr << "multi-fasta-split [options] <in_file>\n";
	cerr << "   in_file               - input multi-FASTA file name (can be gzipped)\n";
	cerr << "Options:\n";
	cerr << "   -n <int>              - no. of sequences in a single output file (default: " << n << ")\n";
	cerr << "   --out-prefix <string> - prefix of output file names (default: " << out_prefix << ")\n";
	cerr << "   --out-suffix <string> - suffix of output file names (default: " << out_suffix << ")\n";
	cerr << "   --part-digits <int>   - no. of digits in part_id (default: " << part_digits << ")\n";
	cerr << "   --gzipped-output      - gzip ouptut files (default: false)\n";
	cerr << "   --gzip-level <int>    - compression level for output gzips (default: " << gzip_level << ")\n";
	cerr << "   --verbosity <int>     - verbosity level (default: " << verbosity << ")\n";
	cerr << "   --remove-empty-lines  - remove empty lines\n";
	cerr << "Example: multi-fasta-split -n 1000 bacteria.fna\n";
}

// **************************************************
string part_fn(int part_id, const int no_digits)
{
	string id(no_digits, '0');
	id += to_string(part_id);
	id = id.substr(id.length() - no_digits, no_digits);

	return out_prefix + "_" + id + "." + out_suffix;
}

// **************************************************
void store_buffer(FILE *f_out)
{
	if (in_buf.empty())
		return;

	if (!gzipped_output)
	{
		auto written = fwrite(in_buf.data(), 1, in_buf.size(), f_out);

		if (written != in_buf.size())
		{
			cerr << "Error in writing" << endl;
			exit(1);
		}

		in_buf.clear();

		return;
	}

	out_buf.resize(in_buf.size() + giz.get_overhead(in_buf.size()));

	auto gzipped_size = giz.compress(in_buf.data(), in_buf.size(), out_buf.data(), out_buf.size(), gzip_level);

	auto written = fwrite(out_buf.data(), 1, gzipped_size, f_out);

	if (written != gzipped_size)
	{
		cerr << "Error in writing" << endl;
		exit(1);
	}

	in_buf.clear();

	return;
}

// **************************************************
void process()
{
	int64_t i = n - 1;
	int part_id = -1;
	string line;


	in_buf.reserve(IN_BUF_SIZE + 1024);
	out_buf.reserve(IN_BUF_SIZE + 1024);

	stream_in_file msgz(in_name);

	if (!msgz.is_open())
		cerr << "Error: cannot open " << in_name << endl;

	stream_decompression sdf(&msgz);

	FILE* f_out = nullptr;

	string out_mode;

	if (gzipped_output)
		out_suffix += ".gz";

	while(!sdf.eof())
	{
		if (sdf.getline(line) == 0)
		{
			if (line.empty())
			{
				if(!remove_empty_lines)
					in_buf.push_back('\n');

				continue;
			}

			if (line.front() == '>' && ++i == n)
			{
				if (f_out)
				{
					store_buffer(f_out);
					fclose(f_out);
				}

				++part_id;
				if(verbosity > 0)
					cerr << part_id << "\r";
				i = 0;

				string out_name = part_fn(part_id, part_digits);
				f_out = fopen(out_name.c_str(), "wb");

				if (!f_out)
				{
					cerr << "Cannot create file: " << out_name << endl;
					exit(1);
				}
			}

			in_buf.insert(in_buf.end(), line.begin(), line.end());
			in_buf.push_back('\n');

			if (in_buf.size() >= IN_BUF_SIZE)
				store_buffer(f_out);
		}
	}

	if (f_out)
	{
		store_buffer(f_out);
		fclose(f_out);
	}
}

// **************************************************
int main(int argc, char** argv)
{
	if (!parse_args(argc, argv))
	{
		usage();
		return 0;
	}

	if (working_mode == working_mode_t::none)
		return 0;

	process();

	return 0;
}
