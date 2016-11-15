/****
Copyright (c) 2016, Benjamin Buchfink
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

#include <set>
#include "tools.h"
#include "../basic/config.h"
#include "../data/sequence_set.h"
#include "../util/seq_file_format.h"
#include "../data/queries.h"
#include "../data/load_seqs.h"
#include "../data/reference.h"

void get_seq()
{
	Database_file db_file;
	db_file.get_seq();
}

void random_seqs()
{
	Database_file db_file;
	db_file.load_seqs();
	cout << "Sequences = " << ref_seqs::get().get_length() << endl;
	std::set<unsigned> n;
	const size_t count = atoi(config.seq_no[0].c_str());
	while (n.size() < count)
		n.insert((rand()*RAND_MAX+rand()) % ref_seqs::get().get_length());
	Compressed_ostream out(config.output_file);
	unsigned j = 0;
	
	std::string s;
	for (std::set<unsigned>::const_iterator i = n.begin(); i != n.end(); ++i) {
		std::stringstream ss;
		ss << '>' << j++ << endl << ref_seqs::get()[*i] << endl;
		s = ss.str();
		out.write(s.data(), s.length());
	}
	out.close();
}