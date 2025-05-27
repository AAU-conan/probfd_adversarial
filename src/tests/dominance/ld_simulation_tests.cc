#include "probfd/dominance/dense_factor_relation.h"
#include "probfd/dominance/dense_label_relation.h"
#include "probfd/dominance/ld_simulation.h"
#include "probfd/dominance/state_dominance_relation.h"
#include "tests/dominance/common.h"


TEST(DominanceDominanceAnalysisTests, simple_truck)
{
    State TruckA(0), TruckB(1);
    State PackageNotPlaced(0), PackageA(1), PackageInTruck(2), PackageB(3);
    Label LoadA(0), LoadB(1), UnloadA(2), UnloadB(3), PlacePackage(4), DriveAB(5), DriveBA(6);

    FTSTASK(
        {
            LTS({
                TR(TruckA, DriveAB, {TruckB}),
                TR(TruckB, DriveBA, {TruckA}),
                TR(TruckA, LoadA, {TruckA}),
                TR(TruckA, UnloadA, {TruckA}),
                TR(TruckB, LoadB, {TruckB}),
                TR(TruckB, UnloadB, {TruckB}),
                TR(TruckA, PlacePackage, {TruckA, TruckA}),
                TR(TruckB, PlacePackage, {TruckB, TruckB})
            }, {TruckA, TruckB}, TruckA),
            LTS({
                TR(PackageNotPlaced, PlacePackage, {PackageA, PackageB}),
                TR(PackageA, LoadA, {PackageInTruck}),
                TR(PackageB, LoadB, {PackageInTruck}),
                TR(PackageInTruck, UnloadA, {PackageA}),
                TR(PackageInTruck, UnloadB, {PackageB}),
                TR(PackageNotPlaced, DriveAB, {PackageNotPlaced}),
                TR(PackageNotPlaced, DriveBA, {PackageNotPlaced}),
                TR(PackageA, DriveAB, {PackageA}),
                TR(PackageA, DriveBA, {PackageA}),
                TR(PackageB, DriveAB, {PackageB}),
                TR(PackageB, DriveBA, {PackageB}),
                TR(PackageInTruck, DriveAB, {PackageInTruck}),
                TR(PackageInTruck, DriveBA, {PackageInTruck})
            }, {PackageA}, PackageNotPlaced)
        },
        COSTS(1, 1, 1, 1, 1, 1, 1),
        OUTCOMES(1, 1, 1, 1, 2, 1, 1)
    );

    LDSimulation ld_sim(std::make_shared<FactorDominanceRelationFactoryImpl<DenseFactorRelation>>(), std::make_shared<LabelRelationFactoryImpl<DenseLabelOutcomeRelation>>());

    auto state_dominance_relation = ld_sim.compute_dominance_relation(fts_task);


    auto& truck_fdr = state_dominance_relation->get_local_relations().at(0);
    auto& package_fdr = state_dominance_relation->get_local_relations().at(1);

    ASSERT_FALSE(truck_fdr->simulates(TruckA, TruckB));
    ASSERT_FALSE(truck_fdr->simulates(TruckB, TruckA));
    ASSERT_TRUE(truck_fdr->simulates(TruckA, TruckA));
    ASSERT_TRUE(truck_fdr->simulates(TruckB, TruckB));

    ASSERT_TRUE(package_fdr->simulates(PackageA, PackageA));
    ASSERT_TRUE(package_fdr->simulates(PackageA, PackageB));
    ASSERT_TRUE(package_fdr->simulates(PackageA, PackageInTruck));
    ASSERT_TRUE(package_fdr->simulates(PackageA, PackageNotPlaced));
    ASSERT_TRUE(package_fdr->simulates(PackageInTruck, PackageInTruck));
    ASSERT_TRUE(package_fdr->simulates(PackageInTruck, PackageB));
    ASSERT_TRUE(package_fdr->simulates(PackageInTruck, PackageNotPlaced));
    ASSERT_TRUE(package_fdr->simulates(PackageB, PackageB));
    ASSERT_TRUE(package_fdr->simulates(PackageB, PackageNotPlaced));
    ASSERT_TRUE(package_fdr->simulates(PackageNotPlaced, PackageNotPlaced));
    ASSERT_FALSE(package_fdr->simulates(PackageB, PackageA));
    ASSERT_FALSE(package_fdr->simulates(PackageB, PackageInTruck));
    ASSERT_FALSE(package_fdr->simulates(PackageInTruck, PackageA));
    ASSERT_FALSE(package_fdr->simulates(PackageNotPlaced, PackageA));
    ASSERT_FALSE(package_fdr->simulates(PackageNotPlaced, PackageB));
    ASSERT_FALSE(package_fdr->simulates(PackageNotPlaced, PackageInTruck));

    auto& label_relation = state_dominance_relation->get_label_relation();

    ASSERT_TRUE(label_relation.noop_dominates_label_in_all_other(FactorIndex(0), fts_task, lom.get_label_outcome(LoadA, 0)));
    ASSERT_TRUE(label_relation.noop_dominates_label_in_all_other(FactorIndex(0), fts_task, lom.get_label_outcome(UnloadB, 0)));
}
