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

#include "query_mapper.h"
#include "../data/reference.h"
#include "../dp/floating_sw.h"

using std::list;

bool is_contained(const vector<Seed_hit>::const_iterator &hits, size_t i)
{
	for (size_t j = 0; j < i; ++j)
		if (hits[i].frame_ == hits[j].frame_ && hits[i].ungapped.is_enveloped(hits[j].ungapped))
			return true;
	return false;
}

bool is_contained(const list<Hsp_data> &hsps, const Seed_hit &hit)
{
	for (list<Hsp_data>::const_iterator i = hsps.begin(); i != hsps.end(); ++i)
		if (hit.frame_ == i->frame && i->pass_through(hit.ungapped))
			return true;
	return false;
}

void Query_mapper::align_target(size_t idx, Statistics &stat)
{
	Target& target = targets[idx];
	std::sort(seed_hits.begin() + target.begin, seed_hits.begin() + target.end);
	const size_t n = target.end - target.begin,
		max_len = query_seq(0).length() + 100 * query_seqs::get().avg_len();
	size_t aligned_len = 0;
	const vector<Seed_hit>::const_iterator hits = seed_hits.begin() + target.begin;

	for (size_t i = 0; i < n; ++i) {
		if (!is_contained(hits, i) && !is_contained(target.hsps, hits[i])) {
			target.hsps.push_back(Hsp_data());
			target.hsps.back().frame = hits[i].frame_;
			uint64_t cell_updates;
			floating_sw(&query_seq(hits[i].frame_)[hits[i].query_pos_],
				&ref_seqs::get()[hits[i].subject_][hits[i].subject_pos_],
				target.hsps.back(),
				config.read_padding(query_seq(hits[i].frame_).length()),
				score_matrix.rawscore(config.gapped_xdrop),
				config.gap_open + config.gap_extend,
				config.gap_extend,
				cell_updates,
				hits[i].query_pos_,
				hits[i].subject_pos_,
				Traceback());
			stat.inc(Statistics::OUT_HITS);
			if (i > 0)
				stat.inc(Statistics::SECONDARY_HITS);
			aligned_len += target.hsps.back().length;
			if (aligned_len > max_len)
				break;
		}
		else
			stat.inc(Statistics::DUPLICATES);
	}

	for (list<Hsp_data>::iterator i = target.hsps.begin(); i != target.hsps.end(); ++i)
		for (list<Hsp_data>::iterator j = target.hsps.begin(); j != target.hsps.end();)
			if (j != i && j->is_weakly_enveloped(*i)) {
				stat.inc(Statistics::ERASED_HITS);
				j = target.hsps.erase(j);
			}
			else
				++j;

	for (list<Hsp_data>::iterator i = target.hsps.begin(); i != target.hsps.end(); ++i)
		i->set_source_range(i->frame, source_query_len);

	target.hsps.sort();
	target.filter_score = target.hsps.front().score;
}
