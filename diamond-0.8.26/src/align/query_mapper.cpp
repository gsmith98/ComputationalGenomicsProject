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

#include "align_queries.h"
#include "query_mapper.h"
#include "../data/reference.h"
#include "extend_ungapped.h"
#include "../output/output.h"
#include "../output/output_format.h"
#include "../output/daa_write.h"

Query_queue query_queue;

Query_mapper::Query_mapper() :
	source_hits(get_query_data()),
	query_id(source_hits.first->query_ / align_mode.query_contexts),
	targets_finished(0),
	next_target(0),
	source_query_len(align_mode.query_translated ? (unsigned)query_seqs::get().reverse_translated_len(query_id*align_mode.query_contexts) : (unsigned)query_seqs::get().length(query_id)),
	seed_hits(source_hits.second - source_hits.first)
{	
}

void Query_mapper::init()
{
	targets.resize(count_targets());
	load_targets();
	rank_targets();
}

pair<Trace_pt_list::iterator, Trace_pt_list::iterator> Query_mapper::get_query_data()
{
	const Trace_pt_list::iterator begin = query_queue.trace_pt_pos;
	if (begin == query_queue.trace_pt_end)
		return pair<Trace_pt_list::iterator, Trace_pt_list::iterator>(begin, begin);
	const unsigned c = align_mode.query_contexts, query = begin->query_ / c;
	Trace_pt_list::iterator end = begin;
	for (; end < query_queue.trace_pt_end && end->query_ / c == query; ++end);
	query_queue.trace_pt_pos = end;
	return pair<Trace_pt_list::iterator, Trace_pt_list::iterator>(begin, end);
}

unsigned Query_mapper::count_targets()
{
	std::sort(source_hits.first, source_hits.second, hit::cmp_subject);
	const size_t n = source_hits.second - source_hits.first;
	const Trace_pt_list::iterator hits = source_hits.first;
	size_t subject_id = std::numeric_limits<size_t>::max();
	unsigned n_subject = 0;
	for (size_t i = 0; i < n; ++i) {
		std::pair<size_t, size_t> l = ref_seqs::data_->local_position(hits[i].subject_);
		if (l.first != subject_id) {
			subject_id = l.first;
			++n_subject;
		}
		const unsigned frame = hits[i].query_ % align_mode.query_contexts;
		seed_hits[i] = Seed_hit(frame,
			(unsigned)l.first,
			(unsigned)l.second,
			hits[i].seed_offset_,
			ungapped_extension((unsigned)l.first,
				(unsigned)l.second,
				hits[i].seed_offset_,
				query_seq(frame)));
	}
	return n_subject;
}

void Query_mapper::load_targets()
{
	unsigned subject_id = std::numeric_limits<unsigned>::max(), n = 0;
	for (size_t i = 0; i < seed_hits.size(); ++i) {
		if (seed_hits[i].subject_ != subject_id) {
			if (n > 0)
				targets[n - 1].end = i;
			targets.get(n) = new Target(i, seed_hits[i].subject_);
			++n;
			subject_id = seed_hits[i].subject_;
		}
		targets[n - 1].filter_score += seed_hits[i].ungapped.score;
	}
	targets[n - 1].end = seed_hits.size();
}

void Query_mapper::rank_targets()
{
	std::sort(targets.begin(), targets.end(), Target::compare);

	unsigned score = 0;
	if (config.toppercent < 100) {
		score = unsigned((double)targets[0].filter_score * (1.0 - config.toppercent / 100.0) * config.rank_ratio);
	}
	else {
		size_t min_idx = std::min(targets.size(), (size_t)config.max_alignments);
		score = unsigned((double)targets[min_idx - 1].filter_score * config.rank_ratio);
	}

	unsigned i = 0;
	for (; i < targets.size(); ++i)
		if (targets[i].filter_score < score)
			break;

	targets.erase(targets.begin() + std::min((unsigned)targets.size(), std::max((unsigned)(config.max_alignments*config.rank_factor), i)), targets.end());
}

bool Query_mapper::generate_output(Text_buffer &buffer, Statistics &stat)
{
	std::sort(targets.begin(), targets.end(), Target::compare);

	unsigned n_hsp = 0, n_target_seq = 0, hit_hsps = 0;

	const unsigned min_raw_score = (unsigned)score_matrix.rawscore(config.min_bit_score == 0
		? score_matrix.bitscore(config.max_evalue, ref_header.letters, (unsigned)query_seq(0).length()) : config.min_bit_score);
	const unsigned top_score = targets[0].filter_score;
	size_t seek_pos = 0;

	for (size_t i = 0; i < targets.size(); ++i) {
		if(targets[i].filter_score < min_raw_score)
			break;
		if (!config.output_range(n_target_seq, targets[i].filter_score, top_score))
			break;
		
		hit_hsps = 0;
		for (list<Hsp_data>::iterator j = targets[i].hsps.begin(); j != targets[i].hsps.end(); ++j) {
			if (j->id_percent() < config.min_id || j->query_cover_percent(source_query_len) < config.query_cover)
				continue;
			if (hit_hsps > 0 && config.single_domain)
				continue;

			if (blocked_processing) {
				if (n_hsp == 0)
					seek_pos = Intermediate_record::write_query_intro(buffer, query_id);
				Intermediate_record::write(buffer, *j, query_id, targets[i].subject_id);
			}
			else {
				if (n_hsp == 0) {
					if (*output_format == Output_format::daa)
						seek_pos = write_daa_query_record(buffer, query_ids::get()[query_id].c_str(), align_mode.query_translated ? query_source_seqs::get()[query_id] : query_seqs::get()[query_id]);
					else
						output_format->print_query_intro(query_id, query_ids::get()[query_id].c_str(), source_query_len, buffer);
				}
				if (*output_format == Output_format::daa)
					write_daa_record(buffer, *j, query_id, targets[i].subject_id);
				else
					output_format->print_match(Hsp_context(*j,
						query_id,
						query_seq(j->frame),
						query_source_seq(),
						query_ids::get()[query_id].c_str(),
						targets[i].subject_id,
						targets[i].subject_id,
						ref_ids::get()[targets[i].subject_id].c_str(),
						(unsigned)ref_seqs::get()[targets[i].subject_id].length(),
						n_target_seq,
						hit_hsps), buffer);
			}

			if(hit_hsps == 0)
				++n_target_seq;
			++n_hsp;
			++hit_hsps;
			if (config.alignment_traceback && j->gap_openings > 0)
				stat.inc(Statistics::GAPPED);
			stat.inc(Statistics::SCORE_TOTAL, j->score);
		}
	}

	if (n_hsp > 0) {
		if (!blocked_processing) {
			if (*output_format == Output_format::daa)
				finish_daa_query_record(buffer, seek_pos);
			else
				output_format->print_query_epilog(buffer);
		}
		else
			Intermediate_record::finish_query(buffer, seek_pos);
	}

	if (!blocked_processing) {
		stat.inc(Statistics::MATCHES, n_hsp);
		stat.inc(Statistics::PAIRWISE, n_target_seq);
		if (n_hsp > 0)
			stat.inc(Statistics::ALIGNED);
	}
	
	return n_hsp > 0;
}