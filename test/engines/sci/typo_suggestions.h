#include <cxxtest/TestSuite.h>

#include "sci/parser/typo_suggestions.h"
#include "common/util.h"

class SciTypoSuggestionsTestSuite : public CxxTest::TestSuite {
public:
	void test_substitution_insertion_deletion_and_transposition() {
		const char *misspellings[] = { "clomb", "clim", "climmb", "clmib" };
		for (uint i = 0; i < ARRAYSIZE(misspellings); ++i) {
			Sci::TypoSuggestionCollector collector(misspellings[i]);
			collector.consider("climb");
			TS_ASSERT_EQUALS(collector.suggestions().size(), 1U);
			if (!collector.suggestions().empty()) {
				TS_ASSERT_EQUALS(collector.suggestions()[0].word, "climb");
				TS_ASSERT_EQUALS(collector.suggestions()[0].distance, 1);
			}
		}
	}

	void test_distance_limit() {
		Sci::TypoSuggestionCollector twoEdits("clomz");
		twoEdits.consider("climb");
		TS_ASSERT_EQUALS(twoEdits.suggestions().size(), 1U);
		if (!twoEdits.suggestions().empty())
			TS_ASSERT_EQUALS(twoEdits.suggestions()[0].distance, 2);

		Sci::TypoSuggestionCollector threeEdits("zzomb");
		threeEdits.consider("climb");
		TS_ASSERT(threeEdits.suggestions().empty());
	}

	void test_exact_match_supersedes_suggestions() {
		Sci::TypoSuggestionCollector collector("climb");
		collector.consider("clime");
		collector.consider("climb");
		collector.consider("clomb");
		TS_ASSERT(collector.suggestions().empty());
	}

	void test_ambiguous_suggestions_are_stable_and_never_selected() {
		Sci::TypoSuggestionCollector collector("clom");
		collector.consider("clot");
		collector.consider("clob");
		collector.consider("clot");
		TS_ASSERT_EQUALS(collector.suggestions().size(), 2U);
		if (collector.suggestions().size() == 2) {
			TS_ASSERT_EQUALS(collector.suggestions()[0].word, "clob");
			TS_ASSERT_EQUALS(collector.suggestions()[1].word, "clot");
		}
	}

	void test_rank_and_limit() {
		Sci::TypoSuggestionCollector collector("clomb");
		const char *dictionary[] = { "clone", "clown", "clomp", "climb", "clombb", "clombed" };
		for (uint i = 0; i < ARRAYSIZE(dictionary); ++i)
			collector.consider(dictionary[i]);
		TS_ASSERT_EQUALS(collector.suggestions().size(), 3U);
		if (collector.suggestions().size() == 3) {
			TS_ASSERT_EQUALS(collector.suggestions()[0].word, "climb");
			TS_ASSERT_EQUALS(collector.suggestions()[1].word, "clombb");
			TS_ASSERT_EQUALS(collector.suggestions()[2].word, "clomp");
		}
	}

	void test_unsupported_corrections() {
		Sci::TypoSuggestionCollector distant("xyz");
		distant.consider("climb");
		TS_ASSERT(distant.suggestions().empty());

		Sci::TypoSuggestionCollector accented("\xe9" "climb");
		accented.consider("climb");
		TS_ASSERT(accented.suggestions().empty());

		Sci::TypoSuggestionCollector punctuation("clomb!");
		punctuation.consider("climb");
		TS_ASSERT(punctuation.suggestions().empty());

		Sci::TypoSuggestionCollector longWord("abcdefghijklmnopqrstuvwxy");
		longWord.consider("abcdefghijklmnopqrstuvwx");
		TS_ASSERT(longWord.suggestions().empty());
	}
};
