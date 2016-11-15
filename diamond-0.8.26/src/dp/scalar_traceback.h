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

#ifndef SCALAR_TRACEBACK_H_
#define SCALAR_TRACEBACK_H_

#include <exception>
#include "../basic/score_matrix.h"

template<typename _score>
struct Scalar_traceback_matrix
{
	Scalar_traceback_matrix(const Growing_buffer<_score> &data, int band):
		data_ (data),
		band_ (band)
	{ }
	int operator()(int col, int row) const
	{ return data_.column(col+1)[row - (data_.center(col+1)-band_)]; }
	bool in_band(int col, int row) const
	{ return row >= data_.center(col+1)-band_ && row <= data_.center(col+1)+band_ && row >= 0 && col >= 0; }
	void print(int col, int row) const
	{
		for(unsigned j=0;j<=row;++j) {
			for(unsigned i=0;i<=col;++i)
				printf("%4i", in_band(i, j) ? this->operator()(i, j) : 0);
			printf("\n");
		}
	}
private:
	const Growing_buffer<_score> &data_;
	const int band_;
};

template<typename _score>
bool have_vgap(const Scalar_traceback_matrix<_score> &dp,
		int i,
		int j,
		int gap_open,
		int gap_extend,
		int &l)
{
	int score = dp(i, j);
	l = 1;
	--j;
	while(dp.in_band(i, j)) {
		if(score == dp(i, j) - gap_open - (l-1)*gap_extend)
			return true;
		--j;
		++l;
	}
	return false;
}

template<typename _score>
bool have_hgap(const Scalar_traceback_matrix<_score> &dp,
		int i,
		int j,
		int gap_open,
		int gap_extend,
		int &l)
{
	int score = dp(i, j);
	l = 1;
	--i;
	while(dp.in_band(i, j)) {
		if(score == dp(i, j) - gap_open - (l-1)*gap_extend)
			return true;
		--i;
		++l;
	}
	return false;
}

template<typename _dir, typename _score>
local_match traceback(const Letter *query,
		const Letter *subject,
		const Growing_buffer<_score> &scores,
		int band,
		int gap_open,
		int gap_extend,
		int i,
		int j,
		int score)
{
	if(i == -1)
		return local_match (0);
	Scalar_traceback_matrix<_score> dp (scores, band);
	//dp.print(i, j);

	local_match l;
	l.query_range.begin_ = 0;
	l.query_range.end_ = j + 1;
	l.subject_range.begin_ = 0;
	l.subject_range.end_ = i + 1;
	l.score = score;

	int gap_len;

	while(i>0 || j>0) {
		const Letter lq = get_dir(query, j, _dir()), ls = get_dir(subject, i, _dir());
		const int match_score = score_matrix(lq, ls);
		//printf("i=%i j=%i score=%i subject=%c query=%c\n",i,j,dp(i, j),Value_traits<_val>::ALPHABET[ls],Value_traits<_val>::ALPHABET[lq]);

		if(dp(i, j) == match_score + dp(i-1, j-1) || dp(i, j) == score_matrix(ls,lq) + dp(i - 1, j - 1)) {		// i==0, j==0 ?
			if (lq == ls) {
				l.transcript.push_back(op_match);
				++l.identities;
				++l.positives;
			}
			else {
				l.transcript.push_back(op_substitution, ls);
				++l.mismatches;
				if (match_score > 0)
					++l.positives;
			}
			--i;
			--j;
			++l.length;			
		} else if (have_hgap(dp, i, j, gap_open, gap_extend, gap_len)) {
			++l.gap_openings;
			l.length += gap_len;
			l.gaps += gap_len;
			for (; gap_len > 0; gap_len--)
				l.transcript.push_back(op_deletion, get_dir(subject, i--, _dir()));
		} else if (have_vgap(dp, i, j, gap_open, gap_extend, gap_len)) {
			++l.gap_openings;
			l.length += gap_len;
			l.gaps += gap_len;
			j -= gap_len;
			l.transcript.push_back(op_insertion, (unsigned)gap_len);
		} else {
			throw std::runtime_error("Traceback error.");
		}
	}

	const Letter lq = get_dir(query, 0, _dir()), ls = get_dir(subject, 0, _dir());
	if (lq == ls) {
		l.transcript.push_back(op_match);
		++l.identities;
	}
	else {
		l.transcript.push_back(op_substitution, ls);
		++l.mismatches;
	}
	++l.length;
	return l;
}

template<typename _dir, typename _score>
local_match traceback(const Letter *query,
		const Letter *subject,
		const Double_buffer<_score> &scores,
		int band,
		int gap_open,
		int gap_extend,
		int i,
		int j,
		int score)
{ return local_match (score); }

#endif /* SCALAR_TRACEBACK_H_ */
