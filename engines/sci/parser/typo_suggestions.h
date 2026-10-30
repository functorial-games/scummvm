/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCI_PARSER_TYPO_SUGGESTIONS_H
#define SCI_PARSER_TYPO_SUGGESTIONS_H

#include "common/array.h"
#include "common/str.h"

namespace Sci {

struct TypoSuggestion {
	Common::String word;
	int distance;
};

/**
 * Collect a small, deterministic set of spelling suggestions.
 * This does not modify the game's vocabulary or parsing results.
 */
class TypoSuggestionCollector {
public:
	explicit TypoSuggestionCollector(const Common::String &word) : _word(word), _exactMatch(false) {}

	void consider(const Common::String &candidate) {
		if (_exactMatch)
			return;

		if (_word == candidate) {
			_exactMatch = true;
			_suggestions.clear();
			return;
		}

		if (!isAsciiWord(_word) || !isAsciiWord(candidate))
			return;

		const int limit = _word.size() < 5 ? 1 : 2;
		const int distance = editDistance(_word, candidate, limit);
		if (distance > limit)
			return;

		uint pos = 0;
		while (pos < _suggestions.size()) {
			const TypoSuggestion &existing = _suggestions[pos];
			if (existing.word == candidate)
				return;
			if (distance < existing.distance ||
				(distance == existing.distance && candidate.compareTo(existing.word) < 0))
				break;
			++pos;
		}

		if (pos >= kMaxSuggestions)
			return;

		TypoSuggestion suggestion = { candidate, distance };
		_suggestions.insert_at(pos, suggestion);
		if (_suggestions.size() > kMaxSuggestions)
			_suggestions.remove_at(kMaxSuggestions);
	}

	const Common::Array<TypoSuggestion> &suggestions() const { return _suggestions; }

private:
	enum { kMaxWordLength = 24, kMaxSuggestions = 3 };

	static bool isAsciiWord(const Common::String &word) {
		if (word.size() < 3 || word.size() > kMaxWordLength)
			return false;
		for (uint i = 0; i < word.size(); ++i)
			if (word[i] < 'a' || word[i] > 'z')
				return false;
		return true;
	}

	// Restricted Damerau-Levenshtein distance, including adjacent swaps.
	static int editDistance(const Common::String &word, const Common::String &candidate, int limit) {
		const int wordLen = word.size();
		const int candidateLen = candidate.size();
		if (wordLen > candidateLen + limit || candidateLen > wordLen + limit)
			return limit + 1;

		int beforePrevious[kMaxWordLength + 1] = { 0 };
		int previous[kMaxWordLength + 1];
		int current[kMaxWordLength + 1];

		for (int j = 0; j <= candidateLen; ++j)
			previous[j] = j;

		for (int i = 1; i <= wordLen; ++i) {
			current[0] = i;
			for (int j = 1; j <= candidateLen; ++j) {
				const int cost = word[i - 1] == candidate[j - 1] ? 0 : 1;
				int best = previous[j] + 1;
				if (current[j - 1] + 1 < best)
					best = current[j - 1] + 1;
				if (previous[j - 1] + cost < best)
					best = previous[j - 1] + cost;
				if (i > 1 && j > 1 &&
					word[i - 1] == candidate[j - 2] &&
					word[i - 2] == candidate[j - 1] &&
					beforePrevious[j - 2] + 1 < best)
					best = beforePrevious[j - 2] + 1;
				current[j] = best;
			}
			for (int j = 0; j <= candidateLen; ++j) {
				beforePrevious[j] = previous[j];
				previous[j] = current[j];
			}
		}

		return previous[candidateLen];
	}

	Common::String _word;
	bool _exactMatch;
	Common::Array<TypoSuggestion> _suggestions;
};

} // End of namespace Sci

#endif // SCI_PARSER_TYPO_SUGGESTIONS_H
