#include <gtest/gtest.h>
#include <nextgen/SigmaPlusProver.h>
#include <nextgen/SigmaPlusVerifier.h>

TEST(test_1_out_of_N, proof_verify)
{
    int N = 16;
    int n = 4;
    int index = 0;

    int m = (int)(log(N) / log(n));;

    secp_primitives::GroupElement g;
    g.randomize();
    std::vector<secp_primitives::GroupElement> h_gens;
    h_gens.resize(n * m);
    for(int i = 0; i < n * m; ++i ){
        h_gens[i].randomize();
    }
    secp_primitives::Scalar v, r;
    v.randomize();
    r.randomize();
    nextgen::SigmaPlusProver<secp_primitives::Scalar,secp_primitives::GroupElement> prover(g,h_gens, n, m);

    std::vector<secp_primitives::GroupElement> commits;
    for(int i = 0; i < N; ++i){
        if(i == index){
            secp_primitives::GroupElement c;
            secp_primitives::Scalar zero(uint64_t(0));
            c = nextgen::NextGenPrimitives<secp_primitives::Scalar,secp_primitives::GroupElement>::double_commit(g, zero, h_gens[0], v, h_gens[1], r);
            commits.push_back(c);

        }
        else{
            commits.push_back(secp_primitives::GroupElement());
            commits[i].randomize();
        }
    }

    nextgen::SigmaPlusProof<secp_primitives::Scalar,secp_primitives::GroupElement> proof;

    prover.proof(commits, index, v, r, proof);

    nextgen::SigmaPlusVerifier<secp_primitives::Scalar,secp_primitives::GroupElement> verifier(g, h_gens, n, m);
   EXPECT_TRUE(verifier.verify(commits, proof));

}


TEST(test_1_out_of_N, proof_serialize_deserialize_verify)
{
    int N = 16;
    int n = 4;
    int index = 0;

    int m = (int)(log(N) / log(n));;

    secp_primitives::GroupElement g;
    g.randomize();
    std::vector<secp_primitives::GroupElement> h_gens;
    h_gens.resize(n * m);
    for(int i = 0; i < n * m; ++i ){
        h_gens[i].randomize();
    }
    secp_primitives::Scalar v, r;
    v.randomize();
    r.randomize();
    nextgen::SigmaPlusProver<secp_primitives::Scalar,secp_primitives::GroupElement> prover(g,h_gens, n, m);

    std::vector<secp_primitives::GroupElement> commits;
    for(int i = 0; i < N; ++i){
        if(i == index){
            secp_primitives::GroupElement c;
            secp_primitives::Scalar zero(uint64_t(0));
            c = nextgen::NextGenPrimitives<secp_primitives::Scalar,secp_primitives::GroupElement>::double_commit(g, zero, h_gens[0], v, h_gens[1], r);
            commits.push_back(c);

        }
        else{
            commits.push_back(secp_primitives::GroupElement());
            commits[i].randomize();
        }
    }

    nextgen::SigmaPlusProof<secp_primitives::Scalar,secp_primitives::GroupElement> proof;

    prover.proof(commits, index, v, r, proof);
    unsigned char data[proof.memoryRequired()];
    proof.serialize(data);
    nextgen::SigmaPlusProof<secp_primitives::Scalar,secp_primitives::GroupElement> new_proof;
    new_proof.deserialize(data, n, m);
    nextgen::SigmaPlusVerifier<secp_primitives::Scalar,secp_primitives::GroupElement> verifier(g, h_gens, n, m);
   EXPECT_TRUE(verifier.verify(commits, new_proof));

}