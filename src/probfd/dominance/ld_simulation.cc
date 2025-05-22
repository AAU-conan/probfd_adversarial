#include "probfd/dominance/ld_simulation.h"

#include "probfd/dominance/factor_dominance_relation.h"
#include "probfd/dominance/fts_task.h"
#include "probfd/dominance/label_relation.h"
#include "probfd/dominance/state_dominance_relation.h"

#include "downward/cli/plugins/plugin.h"
#include "downward/cli/utils/logging_options.h"
#include "downward/utils/markup.h"
#include "downward/utils/timer.h"
#include "probfd/dominance/label_outcome_map.h"

using std::vector;

namespace probfd::dominance {
    std::unique_ptr<StateDominanceRelation> LDSimulation::compute_dominance_relation(const FTSTask &task) {
        return compute_ld_simulation(task);
    }

    std::unique_ptr<StateDominanceRelation> LDSimulation::compute_ld_simulation(const FTSTask & task) {
        downward::utils::Timer t;

        std::vector<std::unique_ptr<FactorDominanceRelation>> local_relations;
        local_relations.reserve(task.get_num_variables());
        for (const auto & lts: task.get_factors()) {
            local_relations.push_back(factor_dominance_relation_factory->create(*lts));
        }

        std::cout << "Initialize label dominance: " << task.get_num_labels() << " labels " << task.get_num_variables() << " systems." << std::endl;

        LabelOutcomeMap label_outcome_map(task);

        std::unique_ptr<LabelOutcomeRelation> label_relation = label_relation_factory->create(task, label_outcome_map);
        // Label relation is updated once before first iteration of local relation updates
        update_label_relation(*label_relation, task, local_relations, label_outcome_map);

        size_t total_size = 0, max_size = 0, total_trsize = 0, max_trsize = 0;
        for (const auto & lts: task.get_factors()) {
            max_size = std::max(max_size, lts->size());
            max_trsize = std::max(max_trsize, lts->num_transitions());
            total_size += lts->size();
            total_trsize += lts->num_transitions();
        }
        std::cout << "Compute LDSim on " << task.get_num_variables() << " LTSs."
                  << " Total factor size: " << total_size
                  << ", total trsize: " << total_trsize
                  << ", max factor size: " << max_size
                  << ", max trsize: " << max_trsize
                  << std::endl;

        std::cout << "Init LDSim in " << t() << ":" << std::flush;
        do {
            for (FactorIndex factor(0); factor < local_relations.size(); ++factor) {
                update_local_relation(factor, task, *label_relation, *(local_relations[factor]), label_outcome_map);
            }
            std::cout << " " << t() << std::flush;
        } while (update_label_relation(*label_relation, task, local_relations, label_outcome_map));
        std::cout << std::endl << "LDSimulation finished: " << t() << std::endl;

#ifndef NDEBUG
        for (const auto& [factor, sim] : std::views::enumerate(local_relations)) {
            sim->dump(task.get_factor(FactorIndex(factor)));
        }
        // log << "Label relation: " << std::endl;
        // label_relation->dump(log, task);
#endif
        return std::make_unique<StateDominanceRelation>(std::move(local_relations), label_relation);
    }

    bool update_local_relation(FactorIndex factor, const FTSTask& fts_task, const LabelOutcomeRelation& label_dominance,
                               FactorDominanceRelation& local_relation, const LabelOutcomeMap& label_outcome_map) {
        bool changes = true;
        bool any_changes = false;
        const LabelledTransitionSystem& lts = fts_task.get_factor(factor);
        while (changes) {
            changes = local_relation.remove_simulations_if([&](State t, State s) {
                //log << "Checking states " << lts->name(s) << " and " << lts->name(t) << endl;
                //Check if really t simulates s
                //for each transition s-l->:
                // a) for all outcomes s-l-o-> s'. t >= s' and l dominated by noop?
                // b) exist t-l'->. for all t-l'-o'-> t'. exists s-l-o-> s'. t' >= s' and (l,o) dominated by (l',o')?
                return lts.applyPostSrc(s, [&](const LTSTransition &trs) {
                    //log << "Checking transition " << s << " to " << trs.target << std::endl;

                    const std::vector<Label> &labels_trs = lts.get_labels(trs.label_group);
                 //   assert(!labels_trs.empty());
                    for (Label label_trs : labels_trs) {
                        //log << "Checking label " << labels_trs[i] << " to " << trs.target << std::endl;
                        bool found = true;
                        for (auto [o, lo] : std::views::enumerate(label_outcome_map.get_label_outcomes(label_trs))) {
                            if (!local_relation.simulates(trs.src, trs.targets.at(o)) && label_dominance.noop_dominates_label_in_all_other(factor, fts_task, lo)) {
                                found = false;
                                break;
                            }
                        }
                        if (!found) {
                            found = lts.applyPostSrc(t, [&](const LTSTransition &trt) {
                                const std::vector<Label> &labels_trt = lts.get_labels(trt.label_group);
                                for (Label label_trt: labels_trt) {
                                    for (auto [o2, lo2] : std::views::enumerate(label_outcome_map.get_label_outcomes(label_trt))) {
                                        for (auto [o, lo] : std::views::enumerate(label_outcome_map.get_label_outcomes(label_trs))) {
                                            if (!local_relation.simulates(trt.targets.at(o2), trs.targets.at(o)) && label_dominance.label_dominates_label_in_all_other(factor, fts_task, lo2, lo)) {
                                                goto o2_good;
                                            }
                                        }
                                        // No o s.t. o2 dominates o, label_trt doesn't work
                                        goto label_trt_bad;
                                        o2_good:;
                                    }
                                    // label_trt is good
                                    return true;
                                    label_trt_bad:;
                                }
                                return false;
                            });
                        }

                        if (!found) {
                            return true;
                        }
                    }

                    return false;
                });
            });
            any_changes |= changes;
        }
        return any_changes;
    }

    bool update_label_relation(LabelOutcomeRelation& label_relation, const FTSTask & task, const std::vector<std::unique_ptr<FactorDominanceRelation>> &sim, const LabelOutcomeMap& label_outcome_map) {
        bool changes = false;
        for (FactorIndex factor(0); factor < task.get_num_variables(); ++factor) {
            changes |= label_relation.update_factor(factor, task, *(sim[factor]), label_outcome_map);
        }
        return changes;
    }

    LDSimulation::LDSimulation(std::shared_ptr<FactorDominanceRelationFactory> factor_dominance_relation_factory, std::shared_ptr<LabelRelationFactory> label_relation_factory) :
            factor_dominance_relation_factory(std::move(factor_dominance_relation_factory)), label_relation_factory(std::move(label_relation_factory)) {
    }


    class LDSimulationFeature
            : public downward::cli::plugins::TypedFeature<DominanceAnalysis, LDSimulation> {
    public:
        LDSimulationFeature() : TypedFeature("ld_simulation") {
            document_title("LDSimulation");

            document_synopsis(
                    "This dominance analysis method implements the algorithm described in the following "
                    "paper:" + downward::utils::format_conference_reference(
                            {"{\'A}lvaro Torralba", "J\"org Hoffmann"},
                            "Simulation-Based Admissible Dominance Pruning",
                            "https://homes.cs.aau.dk/~alto/papers/ijcai15.pdf",
                            "Proceedings of the 24th International Joint Conference on Artificial Intelligence (IJCAI'15)",
                            "1689-1695",
                            "AAAI Press",
                            "2015") + "\n"//TODO (doc): Reference other relevant papers
            );
            document_language_support("action costs", "supported");
            document_language_support("conditional effects", "not supported");
            document_language_support("axioms", "not supported");

            add_option<std::shared_ptr<FactorDominanceRelationFactory>>("fdr",
                                                       "The data structure to store the factor dominance relation",
                                                       "dense_fdr()");
            add_option<std::shared_ptr<LabelRelationFactory>>("lr",
                                                       "The data structure to store the label relation",
                                                       "dense_lr()");

            downward::cli::utils::add_log_options_to_feature(*this);
        }


        virtual std::shared_ptr<LDSimulation> create_component(
                const downward::cli::plugins::Options &opts,
                const downward::utils::Context &) const override {

            return downward::cli::plugins::make_shared_from_arg_tuples<LDSimulation>(
                    opts.get<std::shared_ptr<FactorDominanceRelationFactory>>("fdr"),
                    opts.get<std::shared_ptr<LabelRelationFactory>>("lr"));
        }
    };

static downward::cli::plugins::FeaturePlugin<LDSimulationFeature> _plugin;

}