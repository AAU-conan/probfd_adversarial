#include "probfd/dominance/dense_label_relation.h"

#include "downward/cli/plugins/plugin.h"
#include "probfd/dominance/all_none_factor_index.h"
#include "probfd/dominance/fact_names.h"
#include "probfd/dominance/factor_dominance_relation.h"
#include "probfd/dominance/fts_task.h"

#include <cassert>
#include <print>

namespace probfd::dominance {
    bool DenseLabelOutcomeRelation::simulates(LabelOutcome lo1, LabelOutcome lo2, FactorIndex factor) const {
        assert (lo1 >= 0);
        assert((size_t)lo1 < dominates_in.size());
        assert (lo2 >= 0);
        assert((size_t)lo2 < dominates_in[lo1].size());

        return dominates_in[lo1][lo2].contains(factor);
    }

    bool DenseLabelOutcomeRelation::noop_simulates(LabelOutcome lo, FactorIndex factor) const {
        assert(lo >= 0);
        assert((size_t)lo < dominated_by_noop_in.size());

        return dominated_by_noop_in[lo].contains(factor);
    }

    void DenseLabelOutcomeRelation::set_not_simulates(LabelOutcome lo1, LabelOutcome lo2, FactorIndex factor) {
        //std::cout << "Not simulates: " << l1 << " to " << l2 << " in " << factor << std::endl;
        dominates_in[lo1][lo2].remove(factor);
    }

    bool DenseLabelOutcomeRelation::set_not_simulated_by_irrelevant(LabelOutcome lo, FactorIndex factor) {
        //std::cout << "Not simulated by irrelevant: " << l << " in " << factor << std::endl;

        //Returns if there were changes in dominated_by_noop_in
        simulated_by_irrelevant[lo][factor] = false;
        return dominated_by_noop_in[lo].remove(factor);
    }

    size_t DenseLabelOutcomeRelation::get_num_labels() const
    {
        return num_label_outcomes;
    }

    void DenseLabelOutcomeRelation::dump(
        std::ostream& os,
        const FTSTask& fts_task,
        const LabelOutcomeMap& label_outcome_map) const
    {
        auto fvn = fts_task.get_factor(FactorIndex(0)).fact_value_names;
        for (Label l2(0); l2 < fts_task.get_num_labels(); ++l2) {
            for (auto [o2, lo2] : std::views::enumerate(label_outcome_map.get_label_outcomes(l2))) {
                for (Label l1(0); l1 < fts_task.get_num_labels(); ++l1) {
                    if (l1 == l2) {
                        continue;
                    }
                    for (auto [o1, lo1] : std::views::enumerate(label_outcome_map.get_label_outcomes(l1))) {
                        if (!dominates_in[lo2][lo1].is_none()) {
                            os << std::format("({},{}) dominates ({},{}) in {}",
                                              fvn->get_operator_name(l2),
                                              o2,
                                              fvn->get_operator_name(l1),
                                              o1,
                                              dominates_in[lo2][lo1].to_string()
                            ) << std::endl;
                        }
                    }
                }
            }
        }
        for (Label l1(0); l1 < fts_task.get_num_labels(); ++l1) {
            for (auto [o1, lo1] : std::views::enumerate(label_outcome_map.get_label_outcomes(l1))) {
                if (!dominated_by_noop_in[lo1].is_none()) {
                    os << std::format("noop dominates ({},{}) in {}",
                                      fvn->get_operator_name(l1),
                                      o1,
                                      dominated_by_noop_in[lo1].to_string()
                    ) << std::endl;
                }
            }
        }
    }

    bool DenseLabelOutcomeRelation::label_dominates_label_in_all_other(FactorIndex factor, const FTSTask& /*fts_task*/, LabelOutcome lo1, LabelOutcome lo2) const {
        return dominates_in[lo1][lo2].contains_all_except(factor);
    }

    bool DenseLabelOutcomeRelation::noop_dominates_label_in_all_other(FactorIndex factor, const FTSTask& /*fts_task*/, LabelOutcome lo) const {
        return dominated_by_noop_in[lo].contains_all_except(factor);
    }

    bool DenseLabelOutcomeRelation::update_factor(FactorIndex factor, const FTSTask& fts_task, const FactorDominanceRelation& sim, const LabelOutcomeMap& label_outcome_map) {
        bool changes = false;
        const LabelledTransitionSystem& lts = fts_task.get_factor(factor);
        // A label-outcome (l',o') dominates (l,o) in factor Θ_i if ∀s-(l,o)->s'.∃s-(l',o')->s''. s' <= s''
        // A label has a set of outcomes O(l), and whenever s-l->, then for all o ∈ O(l) there is s-(l,o)->s'
        // <= is <sim> and is a factor-dominance relation for the factor Θ_i
        // i is <factor>
        // O is represented by <label_outcome_map>
        for (LabelGroup lg2: lts.get_relevant_label_groups()) {
            for (Label l2 : lts.get_labels(lg2)) {
                for (auto [o2, lo2] : std::views::enumerate(label_outcome_map.get_label_outcomes(l2))) {
                    // Check if other label outcomes lo1 simulate lo2
                    for (LabelGroup lg_1: lts.get_relevant_label_groups()) {
                        for (Label l1 : lts.get_labels(lg_1)) {
                            for (auto [o1, lo1]: std::views::enumerate(label_outcome_map.get_label_outcomes(l1))) {
                                //std::log << "Check " << l1 << " " << l2 << std::endl;
                                //std::log << "Num transitions: " << factor.get_transitions_label(l1).size()
                                //		    << " " << factor.get_transitions_label(l2).size() << std::endl;
                                //Check if it really simulates
                                //For all s-l2-> exists s-l1-> s.t. for all s-l1-o1->t exists s-l2-o2->t' where t' <= t
                                if (simulates(lo1, lo2, factor)) {
                                    for (const auto &tr2: lts.get_transitions_label(l2)) {
                                        bool found = false;
                                        //TODO (efficiency): for(auto tr2 : factor.get_transitions_for_label_src(l1, tr.src)){
                                        for (const auto &tr1: lts.get_transitions_label(l1)) {
                                            if (tr1.src == tr2.src && sim.simulates(tr1.targets.at(o1), tr2.targets.at(o2))) {
                                                found = true;
                                                break;
                                            }
                                        }
                                        if (!found) {
                                            // std::println("Not ({},{}) dominates ({},{}) in factor {}", lts.label_name(l1), o1, lts.label_name(l2), o2, factor.get());
                                            set_not_simulates(lo1, lo2, factor);
                                            changes = true;
                                            break; //Stop checking trs of l2
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // A label l is irrelevant iff for all s and for all o ∈ O(l) we have s -(l,o)-> s

                    // Does irrelevant label dominate (l,o) in factor?
                    // ∀ s-(l,o)->s'. s' <= s
                    if (simulated_by_irrelevant.at(lo2).at(factor)) {
                        for (auto tr: lts.get_transitions_label(l2)) {
                            if (!sim.simulates(tr.src, tr.targets.at(o2))) {
                                // It does not
                                changes |= set_not_simulated_by_irrelevant(lo2, factor);
                                for (Label l: lts.get_irrelevant_labels()) {
                                    for (LabelOutcome lo : label_outcome_map.get_label_outcomes(l)) {
                                        //std::log << "Not simulated by irrelevant: " << l2  << " in " << i << std::endl;
                                        if (simulates(lo, lo2, factor)) {
                                            set_not_simulates(lo, lo2, factor);
                                            changes = true;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }

                    //Does (l,o) dominate irrelevant label in factor?
                    // ∀ s. ∃s-(l,o)->s'. s <= s'
                    if (simulates_irrelevant.at(lo2).at(factor)) {
                        for (State s(0); s < lts.size(); ++s) {
                            bool found = false;
                            for (const auto &tr: lts.get_transitions_label(l2)) {
                                if (tr.src == s && sim.simulates(tr.targets.at(o2), tr.src)) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                // log << "Not simulates irrelevant: " << l2  << " in " << i << endl;
                                simulates_irrelevant.at(lo2).at(factor) = false;
                                for (Label l: lts.get_irrelevant_labels()) {
                                    for (LabelOutcome lo : label_outcome_map.get_label_outcomes(l)) {
                                        if (simulates(lo2, lo, factor)) {
                                            set_not_simulates(lo2, lo, factor);
                                            changes = true;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        return changes;
    }

    DenseLabelOutcomeRelation::DenseLabelOutcomeRelation(const FTSTask & fts_task, const LabelOutcomeMap& label_outcome_map) : LabelOutcomeRelation(fts_task.get_num_labels()) {
        int num_factors = fts_task.get_num_variables();
        num_label_outcomes = 0;
        for (Label l(0); l < fts_task.get_num_labels(); ++l) {
            num_label_outcomes += label_outcome_map.get_label_outcomes(l).size();
        }

        simulates_irrelevant.resize(num_label_outcomes);
        simulated_by_irrelevant.resize(num_label_outcomes);
        for (int i = 0; i < num_label_outcomes; i++) {
            simulates_irrelevant[i].resize(num_factors, true);
            simulated_by_irrelevant[i].resize(num_factors, true);
        }

        dominates_in.resize(num_label_outcomes);
        dominated_by_noop_in.resize(num_label_outcomes, AllNoneFactorIndex::all_factors());
        for (Label l1(0); l1 < fts_task.get_num_labels(); ++l1) {
            for (LabelOutcome lo1 : label_outcome_map.get_label_outcomes(l1)) {
                dominates_in[lo1].resize(num_label_outcomes, AllNoneFactorIndex::all_factors());
                for (Label l2(0); l2 < fts_task.get_num_labels(); ++l2) {
                    for (LabelOutcome lo2 : label_outcome_map.get_label_outcomes(l2)) {
                        if (fts_task.get_label_cost(l1) > fts_task.get_label_cost(l2)) {
                            dominates_in[lo1][lo2] = AllNoneFactorIndex::no_factors();
                        }
                    }
                }
            }
        }
    }

    using DenseLabelRelationFactory = LabelRelationFactoryImpl<DenseLabelOutcomeRelation>;
    class DenseLabelRelationFactoryFeature final : public downward::cli::plugins::TypedFeature<LabelRelationFactory, DenseLabelRelationFactory> {
    public:
        DenseLabelRelationFactoryFeature() : TypedFeature("dense_lr") {
            document_title("Dense Label Relation");
            document_synopsis("Stores the label relation in a dense matrix with compact factors");
        }

        [[nodiscard]] std::shared_ptr<DenseLabelRelationFactory> create_component(
            const downward::cli::plugins::Options &opts, const downward::utils::Context& context) const override {
            return downward::cli::plugins::make_shared_from_arg_tuples<DenseLabelRelationFactory>();
        }
    };
    static downward::cli::plugins::FeaturePlugin<DenseLabelRelationFactoryFeature> _dense_plugin;
}