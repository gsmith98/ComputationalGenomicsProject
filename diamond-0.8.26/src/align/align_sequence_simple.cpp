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

#include <stddef.h>
#include "../dp/floating_sw.h"
#include "../data/queries.h"
#include "align.h"
#include "../data/reference.h"
#include "match_func.h"

using std::vector;

void align_sequence_simple(vector<Segment> &matches,
	Statistics &stat,
	vector<local_match> &local,
	unsigned *padding,
	size_t db_letters,
	unsigned dna_len,
	Trace_pt_buffer::Vector::iterator &begin,
	Trace_pt_buffer::Vector::iterator &end)
{
	std::sort(begin, end, hit::cmp_normalized_subject);
	const unsigned q_num(begin->query_);
	const sequence query(query_seqs::get()[q_num]);
	const unsigned frame = q_num % align_mode.query_contexts;
	const unsigned query_len = (unsigned)query.length();
	padding[frame] = config.read_padding(query_len);
	uint64_t cell_updates = 0;

	const Sequence_set *ref = ref_seqs::data_;
	for (Trace_pt_buffer::Vector::iterator i = begin; i != end; ++i) {
		if (i != begin && (i->global_diagonal() - (i - 1)->global_diagonal()) <= padding[frame]) {
			std::pair<size_t, size_t> l1 = ref_seqs::data_->local_position((size_t)i->subject_), l2 = ref_seqs::data_->local_position((size_t)(i - 1)->subject_);
			if (l1.first == l2.first) {
				stat.inc(Statistics::DUPLICATES);
				continue;
			}
		}
		local.push_back(local_match(i->seed_offset_, 0, ref->data((ptrdiff_t)i->subject_)));
		floating_sw(&query[i->seed_offset_],
			local.back().subject_,
			local.back(),
			padding[frame],
			score_matrix.rawscore(config.gapped_xdrop),
			config.gap_open + config.gap_extend,
			config.gap_extend,
			cell_updates,
			local.back().query_anchor_,
			local.back().subject_anchor,
			Traceback());
		const int score = local.back().score;
		std::pair<size_t, size_t> l = ref_seqs::data_->local_position((size_t)i->subject_);
		matches.push_back(Segment(score, frame, &local.back(), (unsigned)l.first));

		//local.back().print(query, ref_seqs<_val>::get()[l.first], transcript_buf);

		local.back().set_source_range(frame, dna_len);
		stat.inc(Statistics::OUT_HITS);
	}
}