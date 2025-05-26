#include "probfd/dominance/dense_factor_relation.h"
#include "probfd/dominance/dense_label_relation.h"
#include "probfd/dominance/fact_names.h"
#include "probfd/dominance/fts_task.h"

#include <gtest/gtest.h>

#include "probfd/dominance/labelled_transition_system.h"
#include "probfd/dominance/ld_simulation.h"
#include "probfd/dominance/manual_factor_dominance_relation.h"
#include "probfd/dominance/manual_label_outcome_relation.h"
#include "probfd/dominance/strong_types.h"

using namespace probfd::dominance;

#define FTSTASK(...) \
    auto fn = std::make_shared<NoFactNames>(); \
    auto fvn = std::make_shared<FactValueNames>(fn, -1); \
    FTSTask fts_task(__VA_ARGS__, std::nullopt); \
    LabelOutcomeMap lom(fts_task);

#define LTS(...) \
    LabelledTransitionSystem(__VA_ARGS__, fvn)

#define COSTS(...) \
    {__VA_ARGS__}

#define OUTCOMES(...) \
    {__VA_ARGS__}

#define TR(...) \
    {__VA_ARGS__}

TEST(FactorDominanceTests, simple_noop_dominate)
{
    FTSTASK(
        {
            LabelledTransitionSystem({{0_s, 0_l, {1_s}}}, {1_s}, 0_s, fvn)
        },
        {1},
        {1}
    );

    ManualLabelOutcomeRelation lor(1);
    lor.set_identity();
    lor.set_noop_dominates(LabelOutcome(0), AllNoneFactorIndex::all_factors());
    DenseFactorRelation fr(fts_task.get_factor(FactorIndex(0)));

    update_local_relation(FactorIndex(0), fts_task, lor, fr, lom);

    ASSERT_TRUE(fr.simulates(1_s, 0_s));
    ASSERT_FALSE(fr.simulates(0_s, 1_s));
}

TEST(FactorDominanceTests, simple_label_dominate)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {2_s}),
                TR(1_s, 1_l, {2_s})
            }, {2_s}, 0_s)
        },
        COSTS(1, 1),
        OUTCOMES(1, 1)
    );

    ManualLabelOutcomeRelation lor(lom.get_total_num_label_outcomes());
    lor.set_identity();
    lor.set_dominates(lom.get_label_outcome(0_l, 0), lom.get_label_outcome(1_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));
    DenseFactorRelation fr(fts_task.get_factor(FactorIndex(0)));

    update_local_relation(FactorIndex(0), fts_task, lor, fr, lom);

    ASSERT_TRUE(fr.simulates(0_s, 1_s));
    ASSERT_TRUE(fr.simulates(1_s, 0_s));
}

TEST(FactorDominanceTests, simple_label_dominance)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {1_s}),
                TR(0_s, 1_l, {2_s})
            }, {1_s, 2_s}, 0_s),
            LTS({TR(0_s, 0_l, {0_s}), TR(0_s, 0_l, {0_s})}, {}, 0_s)
        },
        COSTS(1, 1),
        OUTCOMES(1, 1)
    );

    ManualFactorDominanceRelation fdr(fts_task.get_factor(FactorIndex(0)).size());
    fdr.set_identity();
    fdr.set_simulates(1_s, 2_s);

    DenseLabelOutcomeRelation lor(fts_task, lom);

    lor.update_factor(FactorIndex(0), fts_task, fdr, lom);

    ASSERT_TRUE(lor.label_dominates_label_in_all_other(FactorIndex(1), fts_task, lom.get_label_outcome(0_l, 0), lom.get_label_outcome(1_l, 0)));
    ASSERT_FALSE(lor.label_dominates_label_in_all_other(FactorIndex(1), fts_task, lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 0)));
}


TEST(FactorDominanceTests, simple_two_outcome_label_dominance)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {2_s, 3_s}),
                TR(1_s, 1_l, {3_s})
            }, {3_s}, 0_s),
            LTS({TR(0_s, 0_l, {0_s, 0_s}), TR(0_s, 0_l, {0_s})}, {}, 0_s)
        },
        COSTS(1, 1),
        OUTCOMES(2, 1)
    );

    ManualLabelOutcomeRelation lor(lom.get_total_num_label_outcomes());
    lor.set_identity();
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 1), AllNoneFactorIndex::all_except(FactorIndex(0)));

    DenseFactorRelation fr(fts_task.get_factor(FactorIndex(0)));

    update_local_relation(FactorIndex(0), fts_task, lor, fr, lom);

    ASSERT_TRUE(fr.simulates(3_s, 2_s));

    ASSERT_TRUE(fr.simulates(1_s, 0_s));
    ASSERT_FALSE(fr.simulates(0_s, 1_s));
}

TEST(FactorDominanceTests, simple_two_outcome_label_dominance2)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {2_s, 3_s}),
                TR(1_s, 1_l, {3_s})
            }, {2_s}, 0_s),
            LTS({TR(0_s, 0_l, {0_s, 0_s}), TR(0_s, 0_l, {0_s})}, {}, 0_s)
        },
        COSTS(1, 1),
        OUTCOMES(2, 1)
    );

    ManualLabelOutcomeRelation lor(lom.get_total_num_label_outcomes());
    lor.set_identity();
    lor.set_dominates(lom.get_label_outcome(0_l, 0), lom.get_label_outcome(1_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));
    lor.set_dominates(lom.get_label_outcome(0_l, 1), lom.get_label_outcome(1_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));

    DenseFactorRelation fr(fts_task.get_factor(FactorIndex(0)));

    update_local_relation(FactorIndex(0), fts_task, lor, fr, lom);

    ASSERT_TRUE(fr.simulates(2_s, 3_s));

    ASSERT_FALSE(fr.simulates(1_s, 0_s));
    ASSERT_TRUE(fr.simulates(0_s, 1_s));
}

TEST(FactorDominanceTests, simple_two_outcome_label_dominance3)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {2_s, 3_s}),
                TR(1_s, 1_l, {3_s})
            }, {2_s}, 0_s),
            LTS({TR(0_s, 0_l, {0_s, 0_s}), TR(0_s, 0_l, {0_s})}, {}, 0_s)
        },
        COSTS(1, 1),
        OUTCOMES(2, 1)
    );

    ManualLabelOutcomeRelation lor(lom.get_total_num_label_outcomes());
    lor.set_identity();
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 1), AllNoneFactorIndex::all_except(FactorIndex(0)));

    DenseFactorRelation fr(fts_task.get_factor(FactorIndex(0)));

    update_local_relation(FactorIndex(0), fts_task, lor, fr, lom);

    ASSERT_TRUE(fr.simulates(2_s, 3_s));

    ASSERT_TRUE(fr.simulates(1_s, 0_s));
    ASSERT_FALSE(fr.simulates(0_s, 1_s));
}

TEST(FactorDominanceTests, simple_two_outcome_label_dominance4)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {2_s, 3_s}),
                TR(1_s, 1_l, {3_s, 4_s}),
                TR(3_s, 2_l, {3_s}),
                TR(2_s, 2_l, {2_s})
            }, {2_s}, 0_s),
            LTS({TR(0_s, 0_l, {0_s, 0_s}), TR(0_s, 0_l, {0_s})}, {}, 0_s)
        },
        COSTS(1, 1, 1),
        OUTCOMES(2, 2, 1)
    );

    ManualLabelOutcomeRelation lor(lom.get_total_num_label_outcomes());
    lor.set_identity();
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 0), AllNoneFactorIndex::all_except(FactorIndex(0)));
    lor.set_dominates(lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 1), AllNoneFactorIndex::all_except(FactorIndex(0)));

    DenseFactorRelation fr(fts_task.get_factor(FactorIndex(0)));

    update_local_relation(FactorIndex(0), fts_task, lor, fr, lom);

    ASSERT_EQ(lom.get_num_label_outcomes(0_l), 2);
    ASSERT_EQ(lom.get_num_label_outcomes(1_l), 2);
    ASSERT_EQ(lom.get_num_label_outcomes(2_l), 1);

    ASSERT_TRUE(fr.simulates(2_s, 3_s));

    ASSERT_FALSE(fr.simulates(1_s, 0_s));
    ASSERT_FALSE(fr.simulates(0_s, 1_s));
}
