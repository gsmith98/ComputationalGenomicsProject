/****
Copyright (c) 2016, University of Tuebingen, Benjamin Buchfink
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

#ifndef DP_H_
#define DP_H_

#include "../basic/match.h"
#include "../align/align.h"
#include "score_profile.h"

template<typename _score>
void smith_waterman(const Letter *query, local_match &segment, _score gap_open, _score gap_extend, vector<char> &transcript_buf, const _score& = int());
int smith_waterman(const sequence &query, const sequence &subject, unsigned band, unsigned padding, int op, int ep);

int xdrop_ungapped(const Letter *query, const Letter *subject, unsigned seed_len, unsigned &delta, unsigned &len);
int xdrop_ungapped(const Letter *query, const Letter *subject, unsigned &delta, unsigned &len);
int xdrop_ungapped_right(const Letter *query, const Letter *subject, int &len);

void greedy_align(sequence query, sequence subject, const vector<Diagonal_segment> &sh, bool log);
void greedy_align(sequence query, sequence subject, const Diagonal_segment &sh, bool log);
void greedy_align(sequence query, const Long_score_profile &qp, sequence subject, const Diagonal_segment &sh, bool log);
void greedy_align2(sequence query, const Long_score_profile &qp, sequence subject, const vector<Diagonal_segment> &sh, bool log);
void greedy_align(sequence query, const Long_score_profile &qp, sequence subject, int qa, int sa, bool log);

#endif /* FLOATING_SW_H_ */
