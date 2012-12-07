#include "main/refine-etc.cpp"

#include <algorithm>

bool overlaps_something_in(const Pattern &patt, const set<Pattern> &patts) {
  set<Pattern>::const_iterator it = patts.begin();
  for (; it != patts.end(); ++it) {
    if (patt.overlaps(*it)) {
      return true;
    }
  }
  return false;
}

bool implies_something_in(const Pattern &patt, const set<Pattern> &patts) {
  set<Pattern>::const_iterator it = patts.begin();
  for (; it != patts.end(); ++it) {
    if (patt.implies(*it)) {
      return true;
    }
  }
  return false;
}

bool check_preservation_condition(const multiset<Pattern> &cpatts, const multiset<Pattern> &apatts, const set<Pattern> &repls) {
  multiset<Pattern>::const_iterator it = cpatts.begin();
  for (; it != cpatts.end(); ++it) {
    if (!implies_something_in(*it, repls)) {
      break;
    }
  }
  if (it == cpatts.end()) {
    return true;
  }
  it = apatts.begin();
  for (; it != apatts.end(); ++it) {
    if (overlaps_something_in(*it, repls)) {
      return false;
    }
  }
  return true;
}

bool check_matching_condition(const multiset<Pattern> &cpatts, const multiset<Pattern> &npatts, const set<Pattern> &repls) {
  size_t n = 0;
  multiset<Pattern>::const_iterator cit = cpatts.begin();
  for (; cit != cpatts.end(); ++cit) {
    if (!implies_something_in(*cit, repls)) {
      ++n;
      if (n >= 2) {
        return false;
      }
    }
  }
  cit = npatts.begin();
  for (; cit != npatts.end(); ++cit) {
    if (!implies_something_in(*cit, repls)) {
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  vector<RIFRule> rules, refined, pruned;
  set<Pattern> problems, repls;
  read_rules(refined, repls);
  do {
    rules.swap(refined);
    refined.clear();
    problems.clear();
    vector<RIFRule>::const_iterator it = rules.begin();
    for (; it != rules.end(); ++it) {
      multiset<Pattern> patts;
      extract_patterns(*it, patts, patts, patts, patts);
      multiset<Pattern>::const_iterator pit = patts.begin();
      for (; pit != patts.end(); ++pit) {
        if (!implies_something_in(*pit, repls) && overlaps_something_in(*pit, repls)) {
          problems.insert(*pit);
        }
      }
    }
    refine_rules(rules, repls, problems, refined, refined);
  } while (rules.size() != refined.size());
  rules.swap(refined);
  refined.clear();
  vector<RIFRule>::const_iterator it = rules.begin();
  for (; it != rules.end(); ++it) {
    multiset<Pattern> cpatts, apatts, npatts, rpatts;
    extract_patterns(*it, cpatts, apatts, npatts, rpatts);
    if (check_matching_condition(cpatts, npatts, repls)) {
      cpatts.insert(npatts.begin(), npatts.end());
      apatts.insert(rpatts.begin(), rpatts.end());
      if (check_preservation_condition(cpatts, apatts, repls)) {
        refined.push_back(*it);
      } else {
        // TODO should actually refine from the body toward the head
        pruned.push_back(*it);
      }
    } else {
      pruned.push_back(*it);
    }
  }
  print_rules(cout, refined, false);
  print_rules(cout, pruned, true);
}
