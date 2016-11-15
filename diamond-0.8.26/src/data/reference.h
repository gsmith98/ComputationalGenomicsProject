/****
Copyright (c) 2014-2016, University of Tuebingen, Benjamin Buchfink
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****/

#ifndef REFERENCE_H_
#define REFERENCE_H_

#include <memory>
#include <string>
#include <numeric>
#include <limits>
#include "../util/binary_file.h"
#include "sorted_list.h"
#include "../basic/statistics.h"
#include "../data/seed_histogram.h"
#include "../util/hash_table.h"
#include "../util/hash_function.h"
#include "../basic/packed_loc.h"
#include "sequence_set.h"
#include "../util/ptr_vector.h"

using std::auto_ptr;

struct invalid_database_version_exception : public std::exception
{
	virtual const char* what() const throw()
	{
		return "Incompatible database version";
	}
};

struct Reference_header
{
	Reference_header():
		unique_id (0x24af8a415ee186dllu),
		build (Const::build_version),
		db_version(current_db_version),
		sequences (0),
		letters (0)
	{ }
	uint64_t unique_id;
	uint32_t build, db_version;
	uint64_t sequences, letters, pos_array_offset;
#ifdef EXTRA
	Sequence_type sequence_type;
#endif
	enum { current_db_version = 0 };
};

extern Reference_header ref_header;

struct Database_format_exception : public std::exception
{
	virtual const char* what() const throw()
	{ return "Database file is not a DIAMOND database."; }
};

struct Database_file : public Input_stream
{
	Database_file():
		Input_stream (config.database)
	{
		if(this->read(&ref_header, 1) != 1)
			throw Database_format_exception ();
		if(ref_header.unique_id != Reference_header ().unique_id)
			throw Database_format_exception ();
		if(ref_header.build < min_build_required || ref_header.db_version != Reference_header::current_db_version)
			throw invalid_database_version_exception();
#ifdef EXTRA
		if(sequence_type(_val()) != ref_header.sequence_type)
			throw std::runtime_error("Database has incorrect sequence type for current alignment mode.");
#endif
		pos_array_offset = ref_header.pos_array_offset;
	}
	void rewind()
	{
		pos_array_offset = ref_header.pos_array_offset;
	}
	bool load_seqs();
	void get_seq();
	enum { min_build_required = 74 };

	size_t pos_array_offset;
};

void make_db();

struct ref_seqs
{
	static const Sequence_set& get()
	{ return *data_; }
	static Sequence_set& get_nc()
	{ return *data_; }
	static Sequence_set *data_;
};

struct ref_ids
{
	static const String_set<0>& get()
	{ return *data_; }
	static String_set<0> *data_;
};

extern Partitioned_histogram ref_hst;
extern unsigned current_ref_block;
extern bool blocked_processing;

inline size_t max_id_len(const String_set<0> &ids)
{
	size_t max (0);
	for(size_t i=0;i<ids.get_length(); ++i)
		max = std::max(max, find_first_of(ids[i].c_str(), Const::id_delimiters));
	return max;
}

struct Ref_map
{
	Ref_map():
		next_ (0)
	{ }
	void init(unsigned ref_count)
	{
		const unsigned block = current_ref_block;
		if(data_.size() < block+1) {
			data_.resize(block+1);
			data_[block].insert(data_[block].end(), ref_count, std::numeric_limits<unsigned>::max());
		}
	}
	uint32_t get(unsigned block, unsigned i)
	{
		uint32_t n = data_[block][i];
		if(n != std::numeric_limits<unsigned>::max())
			return n;
		{
			mtx_.lock();
			n = data_[block][i];
			if (n != std::numeric_limits<uint32_t>::max()) {
				mtx_.unlock();
				return n;
			}
			n = next_++;
			data_[block][i] = n;
			len_.push_back((uint32_t)ref_seqs::get().length(i));
			if (config.salltitles)
				name_.push_back(new string(ref_ids::get()[i].c_str()));
			else
				name_.push_back(get_str(ref_ids::get()[i].c_str(), Const::id_delimiters));
			mtx_.unlock();
		}
		return n;
	}

	unsigned length(uint32_t i) const
	{
		return len_[i];
	}
	const char* name(uint32_t i) const
	{
		return name_[i].c_str();
	}
	void init_rev_map()
	{
		rev_map_.resize(next_);
		unsigned n = 0;
		for (unsigned i = 0; i < data_.size(); ++i) {
			for (unsigned j = 0; j < data_[i].size(); ++j)
				if (data_[i][j] != std::numeric_limits<uint32_t>::max())
					rev_map_[data_[i][j]] = n + j;
			n += (unsigned)data_[i].size();
		}
	}
	unsigned original_id(unsigned i) const
	{
		return rev_map_[i];
	}
private:
	tthread::mutex mtx_;
	vector<vector<uint32_t> > data_;
	vector<uint32_t> len_;
	Ptr_vector<string> name_;
	vector<uint32_t> rev_map_;
	uint32_t next_;
	friend void finish_daa(Output_stream&);
};

extern Ref_map ref_map;

#endif /* REFERENCE_H_ */
