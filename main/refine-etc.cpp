#include "pattern-etc.cpp"

#define LINE cerr << "[LINE] " << __FILE__ << ':' << __LINE__ << endl

inline
RIFTerm instantiate_term(const RIFTerm &replace, const RIFTerm &varfunc, const RIFTerm &term) {
  return replace.equals(varfunc) ? term : replace;
}

RIFAtomic instantiate_atomic(const RIFAtomic &atom, const RIFTerm &varfunc, const RIFTerm &term) {
  switch (atom.getType()) {
    case ATOM:
    case EXTERNAL: {
      DPtr<RIFTerm> *args = atom.getArgs();
      DPtr<RIFTerm> *newargs;
      NEW(newargs, APtr<RIFTerm>, args->size());
      RIFTerm *arg = args->dptr();
      RIFTerm *end = arg + args->size();
      RIFTerm *newarg = newargs->dptr();
      for (; arg != end; ++arg) {
        *newarg = instantiate_term(*arg, varfunc, term);
        ++newarg;
      }
      args->drop();
      RIFAtomic ret (atom.getPred(), newargs, atom.getType() == EXTERNAL);
      newargs->drop();
      return ret;
    }
    case EQUALITY:
    case MEMBERSHIP:
    case SUBCLASS: {
      RIFTerm t1 = atom.getPart(1);
      RIFTerm t2 = atom.getPart(2);
      t1 = instantiate_term(t1, varfunc, term);
      t2 = instantiate_term(t2, varfunc, term);
      return RIFAtomic(atom.getType(), t1, t2);
    }
    case FRAME: {
      RIFTerm obj = instantiate_term(atom.getObject(), varfunc, term);
      SlotMap slots (RIFTerm::cmplt0);
      TermSet attrs (RIFTerm::cmplt0);
      TermSet vals (RIFTerm::cmplt0);
      atom.getAttrs(attrs);
      TermSet::const_iterator it = attrs.begin();
      for (; it != attrs.end(); ++it) {
        RIFTerm newattr = instantiate_term(*it, varfunc, term);
        atom.getValues(*it, vals);
        TermSet::const_iterator it2 = vals.begin();
        for (; it2 != vals.end(); ++it2) {
          RIFTerm newval = instantiate_term(*it2, varfunc, term);
          slots.insert(pair<RIFTerm, RIFTerm>(newattr, newval));
        }
      }
      return RIFAtomic(obj, slots);
    }
    default: {
      cerr << "[ERROR] Unhandled case at " << __FILE__ << ':' << __LINE__ << endl;
      THROW(TraceableException, "Unhandled case.");
    }
  }
}

RIFCondition instantiate_condition(const RIFCondition &condition, const RIFTerm &varfunc, const RIFTerm &term) {
  switch (condition.getType()) {
    case ATOMIC: {
      return RIFCondition(instantiate_atomic(condition.getAtomic(), varfunc, term));
    }
    case NEGATION: {
      return RIFCondition(instantiate_condition(condition.getSubformula(), varfunc, term), false);
    }
    case CONJUNCTION: {
      DPtr<RIFCondition> *subconds = condition.getSubformulas();
      DPtr<RIFCondition> *newsubconds;
      NEW(newsubconds, APtr<RIFCondition>, subconds->size());
      RIFCondition *subcond = subconds->dptr();
      RIFCondition *end = subcond + subconds->size();
      RIFCondition *newsubcond = newsubconds->dptr();
      for (; subcond != end; ++subcond) {
        *newsubcond = instantiate_condition(*subcond, varfunc, term);
        ++newsubcond;
      }
      RIFCondition newcond = RIFCondition(CONJUNCTION, newsubconds);
      subconds->drop();
      newsubconds->drop();
      return newcond;
    }
    default: {
      cerr << "[ERROR] Unhandled case at " << __FILE__ << ':' << __LINE__ << endl;
      THROW(TraceableException, "Unhandled case.");
    }
  }
}

RIFActionBlock instantiate_action_block(const RIFActionBlock &action_block, const RIFTerm &varfunc, const RIFTerm &term) {
  DPtr<RIFAction> *actions = action_block.getActions();
  DPtr<RIFAction> *newactions;
  NEW(newactions, APtr<RIFAction>, actions->size());
  RIFAction *action = actions->dptr();
  RIFAction *end = action + actions->size();
  RIFAction *newaction = newactions->dptr();
  for (; action != end; ++action) {
    *newaction = RIFAction(action->getType(), instantiate_atomic(action->getTargetAtomic(), varfunc, term));
    ++newaction;
  }
  RIFActionBlock new_action_block(newactions);
  actions->drop();
  newactions->drop();
  return new_action_block;
}

RIFRule instantiate_rule(const RIFRule &rule, const RIFTerm &varfunc, const RIFTerm &term) {
  return RIFRule(instantiate_condition(rule.getCondition(), varfunc, term),
                 instantiate_action_block(rule.getActionBlock(), varfunc, term));
}

RIFRule split_rule(const RIFRule &rule, const NMap &restrictions, vector<RIFRule> &pruned) {
  cerr << "[INFO] Splitting " << rule << endl;
  if (restrictions.empty()) {
    return rule;
  }
  NMap::const_iterator nit = restrictions.begin();
  vector<RIFAtomic> plcatoms;
  for (; nit != restrictions.end(); ++nit) {
    DPtr<RIFTerm> *neqterms;
    NEW(neqterms, APtr<RIFTerm>, nit->second.size());
    RIFTerm *neqterm = neqterms->dptr();
    TSet::const_iterator tit = nit->second.begin();
    for (; tit != nit->second.end(); ++tit) {
      if (nit->first.getType() == FUNCTION) {
        cerr << "[WARNING] MANUAL SANITY CHECK: Instantiating rule (below) by replacing function " << nit->first << " with " << *tit << ".  If the term is not in the range of the function (given the bound arguments of the function, if any), then this renders the results incorrect.\n\t" << rule << endl;
      }
      RIFRule pruned_rule = instantiate_rule(rule, nit->first, *tit);
      cerr << "[INFO] Split off " << pruned_rule << endl;
      pruned.push_back(pruned_rule);
      *neqterm = *tit;
      ++neqterm;
    }
    RIFTerm listarg (neqterms);
    neqterms->drop();
    NEW(neqterms, APtr<RIFTerm>, 2);
    neqterm = neqterms->dptr();
    *neqterm = listarg;
    *(neqterm + 1) = nit->first;
    plcatoms.push_back(RIFAtomic(predListContains(), neqterms, true));
    neqterms->drop();
  }

  vector<RIFAtomic>::const_iterator pit = plcatoms.begin();
  DPtr<RIFCondition> *subconds = NULL;
  RIFCondition *subcond = NULL;
  NEW(subconds, APtr<RIFCondition>, plcatoms.size() + 1);
  subcond = subconds->dptr();
  pit = plcatoms.begin();
  for (; pit != plcatoms.end(); ++pit) {
    *subcond = RIFCondition(*pit);
    *subcond = RIFCondition(*subcond, false);
    ++subcond;
  }
  *subcond = rule.getCondition();
  RIFCondition newcond(CONJUNCTION, subconds);
  subconds->drop();
  RIFRule resulting_rule = RIFRule(newcond, rule.getActionBlock());
  cerr << "[INFO] After splitting, remainder is " << resulting_rule << endl;
  return resulting_rule;
}

void generate_new_restrictions(const Pattern &head, const set<Pattern> &repls, NMap &restrictions, NMap &varasgns) {
  set<Pattern>::const_iterator rit = repls.begin();
  for (; rit != repls.end(); ++rit) {
    if (head.implies(*rit)) {
      cerr << "[ERROR] FAILED SANITY CHECK!  Did not expect implication between:\n\t" << head << "\n\t" << *rit << endl;
      THROW(TraceableException, "FAILED SANITY CHECK!  Did not expect implication.");
    }
    if (head.implies(*rit)) {
      cerr << "[INFO] Not \"pushing down\" " << *rit << " because it is a superset of " << head << endl;
      continue;
    }
    if (!rit->overlaps(head)) {
      cerr << "[INFO] Not \"pushing down\" " << *rit << " because it does not overlap with " << head << endl;
      continue;
    }
    cerr << "[INFO] \"Pushing down\" " << *rit << " on head " << head << endl;
    size_t i;
    for (i = head.atom.startPart(); i < head.atom.getArity(); ++i) {
      RIFTerm t1 = head.atom.getPart(i);
      if (t1.isGround()) {
        cerr << "[INFO] No change to part " << i << " since it is ground " << t1 << endl;
        continue;
      }
      RIFTerm t2 = rit->atom.getPart(i);
      if (t2.isGround()) {
        NMap::iterator nit = restrictions.find(t1);
        if (nit == restrictions.end()) {
          nit = restrictions.insert(pair<RIFTerm, TSet>(t1, TSet(RIFTerm::cmplt0))).first;
        }
        nit->second.insert(t2);
        cerr << "[INFO] Restricting part " << i << " which is variable (or non-ground function) " << t1 << " so that it cannot be bound to " << t2 << endl;
        continue;
      }
      NMap::const_iterator nit1 = head.neqs.find(t1);
      bool empty = nit1 == head.neqs.end() || nit1->second.empty();
      NMap::const_iterator nit2 = rit->neqs.find(t2);
      if (nit2 == rit->neqs.end()) {
        cerr << "[INFO] No change to part " << i << " which is variable (or non-ground function) " << t1 << " because " << t2 << " is unrestricted.  (I'm not sure this is the appropriate action.)" << endl;
        continue;
      }
      TSet::const_iterator tit = nit2->second.begin();
      for (; tit != nit2->second.end(); ++tit) {
        if (empty || nit1->second.count(*tit) <= 0) {
          NMap::iterator vnit = varasgns.find(t1);
          if (vnit == varasgns.end()) {
            vnit = varasgns.insert(pair<RIFTerm, TSet>(t1, TSet(RIFTerm::cmplt0))).first;
          }
          vnit->second.insert(*tit);
        }
      }
    }
  }
}

//void refine_rules(const vector<RIFRule> &rules, const set<Pattern> &repls,
//                  const set<Pattern> &probs, vector<RIFRule> &refined,
//                  vector<RIFRule> &pruned) {
//  vector<RIFRule>::const_iterator rit = rules.begin();
//  for (; rit != rules.end(); ++rit) {
//    NMAP(newrestrictions);
//    NMAP(varasgns);
//    set<Pattern> cpatts, apatts;
//    extract_patterns(*rit, cpatts, apatts, cpatts, apatts);
//    if (apatts.size() > 1) {
//      // TODO maybe one day handle head with multiple patterns; for now, one it sufficient.
//      cerr << "[ERROR] RESULTS ARE INCORRECT WHEN HEAD OF RULE CONTAINS MORE THAN ONE PATTERN: " << *rit << endl;
//      THROW(TraceableException, "Not handling rules with multiple actions in the head.");
//    }
//    set<Pattern>::const_iterator ait = apatts.begin();
//    for (; ait != apatts.end(); ++ait) {
//      if (probs.count(*ait) > 0) {
//        cerr << "[INFO] Generating restrictions for the following rule since " << *ait << " is a problem.\n\t" << *rit << endl;
//        generate_new_restrictions(*ait, repls, newrestrictions, varasgns);
//      }
//    }
//    if (newrestrictions.empty() && varasgns.empty()) {
//      cerr << "[INFO] No need to split rule " << *rit << endl;
//      refined.push_back(*rit);
//    } else {
//      if (!newrestrictions.empty()) {
//        RIFRule intermediate_rule = split_rule(*rit, newrestrictions, pruned);
//        cerr << "[INFO] Split " << *rit << " and keeping " << intermediate_rule << endl;
//        refined.push_back(intermediate_rule);
//      }
//      if (!varasgns.empty()) {
//        RIFRule intermediate_rule = split_rule(*rit, varasgns, refined);
//        cerr << "[INFO] Split " << *rit << " and throwing away " << intermediate_rule << endl;
//        pruned.push_back(intermediate_rule);
//      }
//    }
//  }
//}

template<typename T, typename LT>
bool set_subset_of(const set<T, LT> &s1, const set<T, LT> &s2) {
  typename set<T>::const_iterator it = s1.begin();
  for (; it != s1.end(); ++it) {
    if (s2.count(*it) < 1) {
      return false;
    }
  }
  return true;
}

RIFCondition buildNotPredListContains(DPtr<RIFTerm> *list, const RIFTerm &varfunc) {
  DPtr<RIFTerm> *args;
  NEW(args, APtr<RIFTerm>, 2);
  RIFTerm *arg = args->dptr();
  arg[0] = RIFTerm(list);
  arg[1] = varfunc;
  RIFAtomic ext (predListContains(), args, true);
  args->drop();
  RIFCondition cond(ext);
  return RIFCondition(cond, false);
}

RIFCondition buildNotPredListContains(const RIFTerm &singleton, const RIFTerm &varfunc) {
  DPtr<RIFTerm> *list;
  NEW(list, APtr<RIFTerm>, 1);
  *(list->dptr()) = singleton;
  RIFCondition cond = buildNotPredListContains(list, varfunc);
  list->drop();
  return cond;
}

RIFCondition buildNotPredListContains(const TSet &set, const RIFTerm &varfunc) {
  DPtr<RIFTerm> *list;
  NEW(list, APtr<RIFTerm>, set.size());
  RIFTerm *arg = list->dptr();
  TSet::const_iterator it = set.begin();
  for (; it != set.end(); ++it) {
    *arg = *it;
    ++arg;
  }
  RIFCondition cond = buildNotPredListContains(list, varfunc);
  list->drop();
  return cond;
}

void restrict_rule(const RIFRule &rule, const RIFTerm &varfunc, const TSet &values,
                   vector<RIFRule> &refined, vector<RIFRule> &pruned) {
  DPtr<RIFCondition> *subconds;
  NEW(subconds, APtr<RIFCondition>, 2);
  RIFCondition *subcond = subconds->dptr();
  subcond[0] = buildNotPredListContains(values, varfunc);
  subcond[1] = rule.getCondition();
  RIFCondition newcond (CONJUNCTION, subconds);
  subconds->drop();
  RIFRule newrule (newcond, rule.getActionBlock());
  refined.push_back(newrule);
  TSet::const_iterator it = values.begin();
  for (; it != values.end(); ++it) {
    newrule = instantiate_rule(rule, varfunc, *it);
    pruned.push_back(newrule);
  }
}

void restrict_rule(const RIFRule &rule, const RIFTerm &leftvf,
                   const TSet &left, const TSet &right,
                   vector<RIFRule> &refined, vector<RIFRule> &pruned) {
  TSet::const_iterator rit = right.begin();
  for (; rit != right.end(); ++rit) {
    if (left.count(*rit) >= 1) {
      continue;
    }
    RIFRule newrule = instantiate_rule(rule, leftvf, *rit);
    refined.push_back(newrule);
  }
  if (!set_subset_of(right, left)) {
    DPtr<RIFCondition> *subconds;
    NEW(subconds, APtr<RIFCondition>, 2);
    RIFCondition *subcond = subconds->dptr();
    subcond[0] = buildNotPredListContains(right, leftvf);
    subcond[1] = rule.getCondition();
    RIFCondition newcond (CONJUNCTION, subconds);
    subconds->drop();
    RIFRule newrule (newcond, rule.getActionBlock());
    pruned.push_back(newrule);
  }
}

void refine_rules(const vector<RIFRule> &rules, const set<Pattern> &repls,
                  const set<Pattern> &probs, vector<RIFRule> &refined,
                  vector<RIFRule> &pruned) {
  vector<RIFRule>::const_iterator rit = rules.begin();
  for (; rit != rules.end(); ++rit) {
    multiset<Pattern> cpatts, apatts;
    extract_patterns(*rit, cpatts, apatts, cpatts, apatts);
    if (apatts.size() > 1) {
      // TODO maybe one day handle head with multiple patterns; for now, one it sufficient.
      cerr << "[ERROR] RESULTS ARE INCORRECT WHEN HEAD OF RULE CONTAINS MORE THAN ONE PATTERN: " << *rit << endl;
      THROW(TraceableException, "Not handling rules with multiple actions in the head.");
    } else if (apatts.size() <= 0) {
      refined.push_back(*rit);
      continue;
    }
    Pattern head = *(apatts.begin());
    if (probs.count(head) <= 0) {
      refined.push_back(*rit);
      continue;
    }
    NMAP(restrictions);
    set<Pattern>::const_iterator repit = repls.begin();
    for (; repit != repls.end(); ++repit) {
      if (head.implies(*repit)) {
        cerr << "[ERROR] SANITY CHECK FAILED AT " << __FILE__ << ':' << __LINE__ << ".  SAT formulation should prevent " << head << " from being a problem when " << *repit << " is replicated."  << endl;
        THROW(TraceableException, "SANITY CHECK FAILED.");
      }
      if (!repit->overlaps(head)) {
        continue;
      }
      size_t i;
      for (i = head.atom.startPart(); i <= head.atom.getArity(); ++i) {
        RIFTerm t1 = head.atom.getPart(i);
        if (t1.isGround()) {
          continue;
        }
        RIFTerm t2 = repit->atom.getPart(i);
        if (t2.isGround()) {
          NMap::iterator nit = restrictions.find(t1);
          if (nit == restrictions.end()) {
            nit = restrictions.insert(pair<RIFTerm, TSet>(t1, TSet(RIFTerm::cmplt0))).first;
          }
          nit->second.insert(t2);
          //restrict_rule(*rit, t1, t2, refined, pruned);
          continue;
        }
        NMap::const_iterator nit1 = head.neqs.find(t1);
        NMap::const_iterator nit2 = repit->neqs.find(t2);
        TSET(emptyset);
        restrict_rule(*rit, t1,
                      nit1 == head.neqs.end() ? emptyset : nit1->second,
                      nit2 == repit->neqs.end() ? emptyset : nit2->second,
                      refined, pruned);
      }
    }
    NMap::const_iterator nit = restrictions.begin();
    for (; nit != restrictions.end(); ++nit) {
      restrict_rule(*rit, nit->first, nit->second, refined, pruned);
    }
  }
}

void print_rules(ostream &out, const vector<RIFRule> &rules, const bool ascomment) {
  vector<RIFRule>::const_iterator it = rules.begin();
  for (; it != rules.end(); ++it) {
    if (ascomment) {
      out << "#PRUNED ";
    }
    out << *it << endl;
  }
}

int refine_main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "[ERROR] Correct usage has exactly one argument: " << argv[0] << " <asgn-file>" << endl;
    THROW(TraceableException, "Incorrect use of command.");
  }
  vector<RIFRule> rules;
  set<Pattern> repls, probs;
  read_rules(rules, repls);
  repls.clear();
  ifstream fin (argv[1]);
  string line;
  while (!fin.eof()) {
    getline(fin, line);
    if (line.size() <= 1) {
      break;
    }
    if (line.size() >= 10 && line.substr(0, 10) == string("REPLICATE ")) {
      cout << "#PRAGMA " << line << endl;
      condstr2patt(line.substr(10), repls);
    } else if (line.size() >= 8 && line.substr(0, 8) == string("PROBLEM ")) {
      cout << "#PRAGMA SPLIT " << line.substr(8) << endl;
      condstr2patt(line.substr(8), probs);
    } else {
      cout << "#PRAGMA " << line << endl;
    }
  }
  fin.close();
  cerr << "[INFO] " << repls.size() << " patterns marked for replication.  " << probs.size() << " patterns marked as problems." << endl;
  vector<RIFRule> refined, pruned;
  refine_rules(rules, repls, probs, refined, pruned);
  print_rules(cout, refined, false);
  print_rules(cout, pruned, true);
}
