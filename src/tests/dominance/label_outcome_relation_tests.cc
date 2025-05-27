#include "probfd/dominance/dense_factor_relation.h"
#include "probfd/dominance/dense_label_relation.h"
#include "probfd/dominance/ld_simulation.h"

#include "tests/dominance/common.h"

TEST(DominanceLabelOutcomeRelationTests, simple_label_dominance)
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


TEST(DominanceLabelOutcomeRelationTests, dominance_label_dominance_cost_respecting)
{
    FTSTASK(
        {
            LTS({
                TR(0_s, 0_l, {1_s}),
                TR(0_s, 1_l, {1_s}),
            }, {}, 0_s),
            LTS({TR(0_s, 0_l, {0_s, 0_s}), TR(0_s, 0_l, {0_s})}, {}, 0_s)
        },
        COSTS(1, 2),
        OUTCOMES(1, 1)
    );

    ManualFactorDominanceRelation fdr(fts_task.get_factor(FactorIndex(0)).size());
    fdr.set_identity();

    DenseLabelOutcomeRelation lor(fts_task, lom);

    lor.update_factor(FactorIndex(0), fts_task, fdr, lom);

    ASSERT_TRUE(lor.label_dominates_label_in_all_other(FactorIndex(0), fts_task, lom.get_label_outcome(0_l, 0), lom.get_label_outcome(1_l, 0)));
    ASSERT_FALSE(lor.label_dominates_label_in_all_other(FactorIndex(0), fts_task, lom.get_label_outcome(1_l, 0), lom.get_label_outcome(0_l, 0)));
}