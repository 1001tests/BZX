#ifndef BZX_LIBLELANTUS_RANGE_PROVER_H
#define BZX_LIBLELANTUS_RANGE_PROVER_H

#include "innerproduct_proof_generator.h"

namespace lelantus {
    
class RangeProver {
public:
    RangeProver(
            const GroupElement& g
            , const GroupElement& h1
            , const GroupElement& h2
            , const std::vector<GroupElement>& g_vector
            , const std::vector<GroupElement>& h_vector
            , uint64_t n
            , unsigned int v);

    // commitments are included into transcript if version >= LELANTUS_TX_VERSION_4_5
    void batch_proof(
            const std::vector<Scalar>& v
            , const std::vector<Scalar>& serialNumbers
            , const std::vector<Scalar>& randomness
            , const std::vector<GroupElement>& commitments
            , RangeProof& proof_out);

private:
    GroupElement g;
    GroupElement h1;
    GroupElement h2;
    std::vector<GroupElement> g_;
    std::vector<GroupElement> h_;
    uint64_t n;
    unsigned int version;

};

}//namespace lelantus

#endif //BZX_LIBLELANTUS_RANGE_PROVER_H
