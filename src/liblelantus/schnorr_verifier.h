#ifndef BZX_LIBLELANTUS_SCHNORR_VERIFIER_H
#define BZX_LIBLELANTUS_SCHNORR_VERIFIER_H

#include "lelantus_primitives.h"

namespace lelantus {
    
class SchnorrVerifier {
public:
    //g and h are being kept by reference, be sure it will not be modified from outside
    SchnorrVerifier(const GroupElement& g, const GroupElement& h);

    bool verify(const GroupElement& y, const SchnorrProof& proof);

private:
    const GroupElement& g_;
    const GroupElement& h_;
};

}//namespace lelantus

#endif //BZX_LIBLELANTUS_SCHNORR_VERIFIER_H
