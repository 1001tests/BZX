#include "validation.h"
#include "zerocoin.h"
#include "sigma.h"
#include "timedata.h"
#include "chainparams.h"
#include "util.h"
#include "base58.h"
#include "definition.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#include "sigma/remint.h"

#include <atomic>
#include <sstream>
#include <chrono>

#include <boost/foreach.hpp>

using namespace std;

// Settings
int64_t nTransactionFee = 0;
int64_t nMinimumInputValue = DUST_HARD_LIMIT;

// btzc: add zerocoin init
// zerocoin init
static CBigNum bnTrustedModulus(ZEROCOIN_MODULUS), bnTrustedModulusV2(ZEROCOIN_MODULUS_V2);

// Set up the Zerocoin Params object
uint32_t securityLevel = 80;
libzerocoin::Params *ZCParams = new libzerocoin::Params(bnTrustedModulus, bnTrustedModulus);
libzerocoin::Params *ZCParamsV2 = new libzerocoin::Params(bnTrustedModulusV2, bnTrustedModulus);

static CZerocoinState zerocoinState;

static bool CheckZerocoinSpendSerial(CValidationState &state, const Consensus::Params &params, CZerocoinTxInfo *zerocoinTxInfo, libzerocoin::CoinDenomination denomination, const CBigNum &serial, int nHeight, bool fConnectTip) {
    if (true) {
        // check for zerocoin transaction in this block as well
        if (zerocoinTxInfo && !zerocoinTxInfo->fInfoIsComplete && zerocoinTxInfo->spentSerials.count(serial) > 0)
            return state.DoS(0, error("CTransaction::CheckTransaction() : two or more spends with same serial in the same block"));

        // check for used serials in zerocoinState
        if (zerocoinState.IsUsedCoinSerial(serial)) {
            // Proceed with checks ONLY if we're accepting tx into the memory pool or connecting block to the existing blockchain
            if (nHeight == INT_MAX || fConnectTip) {
                    return state.DoS(0, error("CTransaction::CheckTransaction() : The CoinSpend serial has been used"));
            }
        }
    }

    return true;
}

CBigNum ParseZerocoinMintScript(const CScript& script)
{
    if (script.size() < 6) {
        throw std::invalid_argument("Script is not a valid Zerocoin mint");
    }

    return CBigNum(std::vector<unsigned char>(script.begin() + 6, script.end()));
}

std::pair<std::unique_ptr<libzerocoin::CoinSpend>, uint32_t> ParseZerocoinSpend(const CTxIn& in)
{
    // Check arguments.
    uint32_t groupId = in.nSequence;

    if (groupId < 1 || groupId >= INT_MAX) {
        throw CBadSequence();
    }

    if (in.scriptSig.size() < 4) {
        throw CBadTxIn();
    }

    // Determine if version 2 spend.
    bool v2 = groupId >= ZC_MODULUS_V2_BASE_ID;

    // Deserialize spend.
    CDataStream serialized(
        std::vector<unsigned char>(in.scriptSig.begin() + 4, in.scriptSig.end()),
        SER_NETWORK,
        PROTOCOL_VERSION
    );

    std::unique_ptr<libzerocoin::CoinSpend> spend(new libzerocoin::CoinSpend(v2 ? ZCParamsV2 : ZCParams, serialized));

    return std::make_pair(std::move(spend), groupId);
}

bool CheckZerocoinFoundersInputs(const CTransaction &tx, CValidationState &state, const Consensus::Params &params, int nHeight) {

    if (nHeight > 450000) {
            bool found_1 = false;
            bool found_2 = false;
            CScript FOUNDER_1_SCRIPT;
            CScript FOUNDER_2_SCRIPT;
            FOUNDER_1_SCRIPT = GetScriptForDestination(CBitcoinAddress("XSgBGYizQrSk4mi79Myqs6xcG9B3gd2m9H").Get());
            FOUNDER_2_SCRIPT = GetScriptForDestination(CBitcoinAddress("XXQYi4i4PVaz6iMs43fi2hz64wFzuY3c6S").Get());
            BOOST_FOREACH(const CTxOut &output, tx.vout)
            {
                if (output.scriptPubKey == FOUNDER_1_SCRIPT && output.nValue == (int64_t)(8 * COIN))
                {
                    found_1 = true;
                    continue;
                }

                if (output.scriptPubKey == FOUNDER_2_SCRIPT && output.nValue == (int64_t)(8 * COIN))
                {
                    found_2 = true;
                    continue;
                }
            }


            if (!(found_1 && found_2))
            {
                return state.DoS(100, false, REJECT_FOUNDER_REWARD_MISSING,
                                 "CTransaction::CheckTransaction() : founders reward missing");
            }
    }
    return true;

}

void DisconnectTipZC(CBlock & /*block*/, CBlockIndex *pindexDelete) {
    zerocoinState.RemoveBlock(pindexDelete);
}

CBigNum ZerocoinGetSpendSerialNumber(const CTransaction &tx, const CTxIn &txin) {
    if (!txin.IsZerocoinSpend())
        return CBigNum(0);
    try {
        CDataStream serializedCoinSpend((const char *)&*(txin.scriptSig.begin() + 4),
                                    (const char *)&*txin.scriptSig.end(),
                                    SER_NETWORK, PROTOCOL_VERSION);
        libzerocoin::CoinSpend spend(txin.nSequence >= ZC_MODULUS_V2_BASE_ID ? ZCParamsV2 : ZCParams, serializedCoinSpend);
        return spend.getCoinSerialNumber();
    }
    catch (const std::runtime_error &) {
        return CBigNum(0);
    }
}

/**
 * Connect a new ZCblock to chainActive. pblock is either NULL or a pointer to a CBlock
 * corresponding to pindexNew, to bypass loading it again from disk.
 */
bool ConnectBlockZC(CValidationState &state, const CChainParams &chainParams, CBlockIndex *pindexNew, const CBlock *pblock, bool fJustCheck) {

    // Add zerocoin transaction information to index
    if (pblock && pblock->zerocoinTxInfo) {
        if (pblock->zerocoinTxInfo->fHasSpendV1) {
            if (true) {
                LogPrintf("ConnectTipZC: spend v1 is not allowed after block %d\n");
                return false;
            }
        }

	    if (!fJustCheck) {
            // clear the state
			pindexNew->spentSerials.clear();
            pindexNew->mintedPubCoins.clear();
            pindexNew->accumulatorChanges.clear();
            pindexNew->alternativeAccumulatorChanges.clear();
        }

        if (true) {
            BOOST_FOREACH(const PAIRTYPE(CBigNum,int) &serial, pblock->zerocoinTxInfo->spentSerials) {
                if (!CheckZerocoinSpendSerial(state, chainParams.GetConsensus(), pblock->zerocoinTxInfo.get(), (libzerocoin::CoinDenomination)serial.second, serial.first, pindexNew->nHeight, true))
                    return false;

                if (!fJustCheck) {
                    pindexNew->spentSerials.insert(serial.first);
                    zerocoinState.AddSpend(serial.first);
                }

            }
        }

        if (fJustCheck)
            return true;

        // Update minted values and accumulators
        BOOST_FOREACH(const PAIRTYPE(int,CBigNum) &mint, pblock->zerocoinTxInfo->mints) {
            CBigNum oldAccValue(0);
            int denomination = mint.first;
            int mintId = zerocoinState.AddMint(pindexNew, denomination, mint.second, oldAccValue);

            libzerocoin::Params *zcParams = IsZerocoinTxV2((libzerocoin::CoinDenomination)denomination,
                                                chainParams.GetConsensus(), mintId) ? ZCParamsV2 : ZCParams;

            if (!oldAccValue)
                oldAccValue = zcParams->accumulatorParams.accumulatorBase;

            LogPrintf("ConnectTipZC: mint added denomination=%d, id=%d\n", denomination, mintId);
            pair<int,int> denomAndId = make_pair(denomination, mintId);

            pindexNew->mintedPubCoins[denomAndId].push_back(mint.second);

            CZerocoinState::CoinGroupInfo coinGroupInfo;
            zerocoinState.GetCoinGroupInfo(denomination, mintId, coinGroupInfo);

            libzerocoin::PublicCoin pubCoin(zcParams, mint.second, (libzerocoin::CoinDenomination)denomination);
            libzerocoin::Accumulator accumulator(zcParams,
                                                 oldAccValue,
                                                 (libzerocoin::CoinDenomination)denomination);
            accumulator += pubCoin;

            if (pindexNew->accumulatorChanges.count(denomAndId) > 0) {
                pair<CBigNum,int> &accChange = pindexNew->accumulatorChanges[denomAndId];
                accChange.first = accumulator.getValue();
                accChange.second++;
            }
            else {
                pindexNew->accumulatorChanges[denomAndId] = make_pair(accumulator.getValue(), 1);
            }
            // invalidate alternative accumulator value for this denomination and id
            pindexNew->alternativeAccumulatorChanges.erase(denomAndId);
        }
    }
    else if (!fJustCheck) {
        zerocoinState.AddBlock(pindexNew, chainParams.GetConsensus());
    }

    return true;
}

int ZerocoinGetNHeight(const CBlockHeader &block) {
    CBlockIndex *pindexPrev = NULL;
    int nHeight = 0;
    BlockMap::iterator mi = mapBlockIndex.find(block.hashPrevBlock);
    if (mi != mapBlockIndex.end()) {
        pindexPrev = (*mi).second;
        nHeight = pindexPrev->nHeight + 1;
    }
    return nHeight;
}


bool ZerocoinBuildStateFromIndex(CChain *chain, set<CBlockIndex *> &changes) {
    auto params = Params().GetConsensus();

    zerocoinState.Reset();
    for (CBlockIndex *blockIndex = chain->Genesis(); blockIndex; blockIndex=chain->Next(blockIndex))
        zerocoinState.AddBlock(blockIndex, params);

    changes = zerocoinState.RecalculateAccumulators(chain);

    // DEBUG
    LogPrintf("Latest IDs are %d, %d, %d, %d, %d\n",
              zerocoinState.latestCoinIds[1],
               zerocoinState.latestCoinIds[10],
            zerocoinState.latestCoinIds[25],
            zerocoinState.latestCoinIds[50],
            zerocoinState.latestCoinIds[100]);
    return true;
}

// CZerocoinTxInfo

void CZerocoinTxInfo::Complete() {
    // We need to sort mints lexicographically by serialized value of pubCoin. That's the way old code
    // works, we need to stick to it. Denomination doesn't matter but we will sort by it as well
    sort(mints.begin(), mints.end(),
         [](decltype(mints)::const_reference m1, decltype(mints)::const_reference m2)->bool {
            CDataStream ds1(SER_DISK, CLIENT_VERSION), ds2(SER_DISK, CLIENT_VERSION);
            ds1 << m1.second;
            ds2 << m2.second;
            return (m1.first < m2.first) || ((m1.first == m2.first) && (ds1.str() < ds2.str()));
         });

    // Mark this info as complete
    fInfoIsComplete = true;
}

// CZerocoinState::CBigNumHash

std::size_t CZerocoinState::CBigNumHash::operator ()(const CBigNum &bn) const noexcept {
    // we are operating on almost random big numbers and least significant bytes (save for few last bytes) give us a good hash
    vector<unsigned char> bnData = bn.ToBytes();
    if (bnData.size() < sizeof(size_t)*3)
        // rare case, put ones like that into one hash bin
        return 0;
    else
        return ((size_t*)bnData.data())[1];
}

// CZerocoinState

CZerocoinState::CZerocoinState() {
}

int CZerocoinState::AddMint(CBlockIndex *index, int denomination, const CBigNum &pubCoin, CBigNum &previousAccValue) {

    int mintId = 1;

    if (latestCoinIds[denomination] < 1)
        latestCoinIds[denomination] = 1;
    else
        mintId = latestCoinIds[denomination];

    // There is a limit of 10 coins per group but mints belonging to the same block must have the same id thus going
    // beyond 10
    CoinGroupInfo &coinGroup = coinGroups[make_pair(denomination, mintId)];
    int coinsPerId = IsZerocoinTxV2((libzerocoin::CoinDenomination)denomination,
                        Params().GetConsensus(), mintId) ? ZC_SPEND_V2_COINSPERID : ZC_SPEND_V1_COINSPERID;
    if (coinGroup.nCoins < coinsPerId || coinGroup.lastBlock == index) {
        if (coinGroup.nCoins++ == 0) {
            // first group of coins for given denomination
            coinGroup.firstBlock = coinGroup.lastBlock = index;
        }
        else {
            previousAccValue = coinGroup.lastBlock->accumulatorChanges[make_pair(denomination,mintId)].first;
            coinGroup.lastBlock = index;
        }
    }
    else {
        latestCoinIds[denomination] = ++mintId;
        CoinGroupInfo &newCoinGroup = coinGroups[make_pair(denomination, mintId)];
        newCoinGroup.firstBlock = newCoinGroup.lastBlock = index;
        newCoinGroup.nCoins = 1;
    }

    CMintedCoinInfo coinInfo;
    coinInfo.denomination = denomination;
    coinInfo.id = mintId;
    coinInfo.nHeight = index->nHeight;
    mintedPubCoins.insert(pair<CBigNum,CMintedCoinInfo>(pubCoin, coinInfo));

    return mintId;
}

void CZerocoinState::AddSpend(const CBigNum &serial) {
    usedCoinSerials.insert(serial);
}

void CZerocoinState::AddBlock(CBlockIndex *index, const Consensus::Params &params) {
    BOOST_FOREACH(const PAIRTYPE(PAIRTYPE(int,int), PAIRTYPE(CBigNum,int)) &accUpdate, index->accumulatorChanges)
    {
        CoinGroupInfo   &coinGroup = coinGroups[accUpdate.first];

        if (coinGroup.firstBlock == NULL)
            coinGroup.firstBlock = index;
        coinGroup.lastBlock = index;
        coinGroup.nCoins += accUpdate.second.second;
    }

    BOOST_FOREACH(const PAIRTYPE(PAIRTYPE(int,int),vector<CBigNum>) &pubCoins, index->mintedPubCoins) {
        latestCoinIds[pubCoins.first.first] = pubCoins.first.second;
        BOOST_FOREACH(const CBigNum &coin, pubCoins.second) {
            CMintedCoinInfo coinInfo;
            coinInfo.denomination = pubCoins.first.first;
            coinInfo.id = pubCoins.first.second;
            coinInfo.nHeight = index->nHeight;
            mintedPubCoins.insert(pair<CBigNum,CMintedCoinInfo>(coin, coinInfo));
        }
    }

    if (true) {
        BOOST_FOREACH(const CBigNum &serial, index->spentSerials) {
            usedCoinSerials.insert(serial);
        }
    }
}

void CZerocoinState::RemoveBlock(CBlockIndex *index) {
    // roll back accumulator updates
    BOOST_FOREACH(const PAIRTYPE(PAIRTYPE(int,int), PAIRTYPE(CBigNum,int)) &accUpdate, index->accumulatorChanges)
    {
        CoinGroupInfo   &coinGroup = coinGroups[accUpdate.first];
        int  nMintsToForget = accUpdate.second.second;

        assert(coinGroup.nCoins >= nMintsToForget);

        if ((coinGroup.nCoins -= nMintsToForget) == 0) {
            // all the coins of this group have been erased, remove the group altogether
            coinGroups.erase(accUpdate.first);
            // decrease pubcoin id for this denomination
            latestCoinIds[accUpdate.first.first]--;
        }
        else {
            // roll back lastBlock to previous position
            do {
                assert(coinGroup.lastBlock != coinGroup.firstBlock);
                coinGroup.lastBlock = coinGroup.lastBlock->pprev;
            } while (coinGroup.lastBlock->accumulatorChanges.count(accUpdate.first) == 0);
        }
    }

    // roll back mints
    BOOST_FOREACH(const PAIRTYPE(PAIRTYPE(int,int),vector<CBigNum>) &pubCoins, index->mintedPubCoins) {
        BOOST_FOREACH(const CBigNum &coin, pubCoins.second) {
            auto coins = mintedPubCoins.equal_range(coin);
            auto coinIt = find_if(coins.first, coins.second, [=](const decltype(mintedPubCoins)::value_type &v) {
                return v.second.denomination == pubCoins.first.first &&
                        v.second.id == pubCoins.first.second;
            });
            assert(coinIt != coins.second);
            mintedPubCoins.erase(coinIt);
        }
    }

    // roll back spends
    BOOST_FOREACH(const CBigNum &serial, index->spentSerials) {
        usedCoinSerials.erase(serial);
    }
}

bool CZerocoinState::GetCoinGroupInfo(int denomination, int id, CoinGroupInfo &result) {
    pair<int,int>   key = make_pair(denomination, id);
    if (coinGroups.count(key) == 0)
        return false;

    result = coinGroups[key];
    return true;
}

bool CZerocoinState::IsUsedCoinSerial(const CBigNum &coinSerial) {
    return usedCoinSerials.count(coinSerial) != 0;
}

bool CZerocoinState::HasCoin(const CBigNum &pubCoin) {
    return mintedPubCoins.count(pubCoin) != 0;
}

int CZerocoinState::GetAccumulatorValueForSpend(CChain *chain, int maxHeight, int denomination, int id,
                                                CBigNum &accumulator, uint256 &blockHash, bool useModulusV2) {

    pair<int, int> denomAndId = pair<int, int>(denomination, id);

    if (coinGroups.count(denomAndId) == 0)
        return 0;

    CoinGroupInfo coinGroup = coinGroups[denomAndId];
    CBlockIndex *lastBlock = coinGroup.lastBlock;

    assert(lastBlock->accumulatorChanges.count(denomAndId) > 0);
    assert(coinGroup.firstBlock->accumulatorChanges.count(denomAndId) > 0);

    // is native modulus for denomination and id v2?
    bool nativeModulusIsV2 = IsZerocoinTxV2((libzerocoin::CoinDenomination)denomination, Params().GetConsensus(), id);
    // field in the block index structure for accesing accumulator changes
    decltype(&CBlockIndex::accumulatorChanges) accChangeField;
    if (nativeModulusIsV2 != useModulusV2) {
        CalculateAlternativeModulusAccumulatorValues(chain, denomination, id);
        accChangeField = &CBlockIndex::alternativeAccumulatorChanges;
    }
    else {
        accChangeField = &CBlockIndex::accumulatorChanges;
    }

    int numberOfCoins = 0;
    for (;;) {
        map<pair<int,int>, pair<CBigNum,int>> &accumulatorChanges = lastBlock->*accChangeField;
        if (accumulatorChanges.count(denomAndId) > 0) {
            if (lastBlock->nHeight <= maxHeight) {
                if (numberOfCoins == 0) {
                    // latest block satisfying given conditions
                    // remember accumulator value and block hash
                    accumulator = accumulatorChanges[denomAndId].first;
                    blockHash = lastBlock->GetBlockHash();
                }
                numberOfCoins += accumulatorChanges[denomAndId].second;
            }
        }

        if (lastBlock == coinGroup.firstBlock)
            break;
        else
            lastBlock = lastBlock->pprev;
    }

    return numberOfCoins;
}

libzerocoin::AccumulatorWitness CZerocoinState::GetWitnessForSpend(CChain *chain, int maxHeight, int denomination,
                                                                   int id, const CBigNum &pubCoin, bool useModulusV2) {

    libzerocoin::CoinDenomination d = (libzerocoin::CoinDenomination)denomination;
    pair<int, int> denomAndId = pair<int, int>(denomination, id);

    assert(coinGroups.count(denomAndId) > 0);

    CoinGroupInfo coinGroup = coinGroups[denomAndId];

    int coinId;
    int mintHeight = GetMintedCoinHeightAndId(pubCoin, denomination, coinId);

    assert(coinId == id);

    libzerocoin::Params *zcParams = useModulusV2 ? ZCParamsV2 : ZCParams;
    bool nativeModulusIsV2 = IsZerocoinTxV2((libzerocoin::CoinDenomination)denomination, Params().GetConsensus(), id);
    decltype(&CBlockIndex::accumulatorChanges) accChangeField;
    if (nativeModulusIsV2 != useModulusV2) {
        CalculateAlternativeModulusAccumulatorValues(chain, denomination, id);
        accChangeField = &CBlockIndex::alternativeAccumulatorChanges;
    }
    else {
        accChangeField = &CBlockIndex::accumulatorChanges;
    }

    // Find accumulator value preceding mint operation
    CBlockIndex *mintBlock = (*chain)[mintHeight];
    CBlockIndex *block = mintBlock;
    libzerocoin::Accumulator accumulator(zcParams, d);
    if (block != coinGroup.firstBlock) {
        do {
            block = block->pprev;
        } while ((block->*accChangeField).count(denomAndId) == 0);
        accumulator = libzerocoin::Accumulator(zcParams, (block->*accChangeField)[denomAndId].first, d);
    }

    // Now add to the accumulator every coin minted since that moment except pubCoin
    block = coinGroup.lastBlock;
    for (;;) {
        if (block->nHeight <= maxHeight && block->mintedPubCoins.count(denomAndId) > 0) {
            vector<CBigNum> &pubCoins = block->mintedPubCoins[denomAndId];
            for (const CBigNum &coin: pubCoins) {
                if (block != mintBlock || coin != pubCoin)
                    accumulator += libzerocoin::PublicCoin(zcParams, coin, d);
            }
        }
        if (block != mintBlock)
            block = block->pprev;
        else
            break;
    }

    return libzerocoin::AccumulatorWitness(zcParams, accumulator, libzerocoin::PublicCoin(zcParams, pubCoin, d));
}

int CZerocoinState::GetMintedCoinHeightAndId(const CBigNum &pubCoin, int denomination, int &id) {
    auto coins = mintedPubCoins.equal_range(pubCoin);
    auto coinIt = find_if(coins.first, coins.second,
                          [=](const decltype(mintedPubCoins)::value_type &v) { return v.second.denomination == denomination; });

    if (coinIt != coins.second) {
        id = coinIt->second.id;
        return coinIt->second.nHeight;
    }
    else
        return -1;
}

void CZerocoinState::CalculateAlternativeModulusAccumulatorValues(CChain *chain, int denomination, int id) {
    libzerocoin::CoinDenomination d = (libzerocoin::CoinDenomination)denomination;
    pair<int, int> denomAndId = pair<int, int>(denomination, id);
    libzerocoin::Params *altParams = IsZerocoinTxV2(d, Params().GetConsensus(), id) ? ZCParams : ZCParamsV2;
    libzerocoin::Accumulator accumulator(altParams, d);

    if (coinGroups.count(denomAndId) == 0) {
        // Can happen when verification is done prior to syncing with network
        return;
    }

    CoinGroupInfo coinGroup = coinGroups[denomAndId];

    CBlockIndex *block = coinGroup.firstBlock;
    for (;;) {
        if (block->accumulatorChanges.count(denomAndId) > 0) {
            if (block->alternativeAccumulatorChanges.count(denomAndId) > 0)
                // already calculated, update accumulator with cached value
                accumulator = libzerocoin::Accumulator(altParams, block->alternativeAccumulatorChanges[denomAndId].first, d);
            else {
                // re-create accumulator changes with alternative params
                assert(block->mintedPubCoins.count(denomAndId) > 0);
                const vector<CBigNum> &mintedCoins = block->mintedPubCoins[denomAndId];
                BOOST_FOREACH(const CBigNum &c, mintedCoins) {
                    accumulator += libzerocoin::PublicCoin(altParams, c, d);
                }
                block->alternativeAccumulatorChanges[denomAndId] = make_pair(accumulator.getValue(), (int)mintedCoins.size());
            }
        }

        if (block != coinGroup.lastBlock)
            block = (*chain)[block->nHeight+1];
        else
            break;
    }
}

bool CZerocoinState::TestValidity(CChain *chain) {
    BOOST_FOREACH(const PAIRTYPE(PAIRTYPE(int,int), CoinGroupInfo) &coinGroup, coinGroups) {
        fprintf(stderr, "TestValidity[denomination=%d, id=%d]\n", coinGroup.first.first, coinGroup.first.second);

        libzerocoin::Params *zcParams = ZCParamsV2;

        libzerocoin::Accumulator acc(&zcParams->accumulatorParams, (libzerocoin::CoinDenomination)coinGroup.first.first);

        CBlockIndex *block = coinGroup.second.firstBlock;
        for (;;) {
            if (block->accumulatorChanges.count(coinGroup.first) > 0) {
                if (block->mintedPubCoins.count(coinGroup.first) == 0) {
                    fprintf(stderr, "  no minted coins\n");
                    return false;
                }

                BOOST_FOREACH(const CBigNum &pubCoin, block->mintedPubCoins[coinGroup.first]) {
                    acc += libzerocoin::PublicCoin(zcParams, pubCoin, (libzerocoin::CoinDenomination)coinGroup.first.first);
                }

                if (acc.getValue() != block->accumulatorChanges[coinGroup.first].first) {
                    fprintf (stderr, "  accumulator value mismatch at height %d\n", block->nHeight);
                    return false;
                }

                if (block->accumulatorChanges[coinGroup.first].second != (int)block->mintedPubCoins[coinGroup.first].size()) {
                    fprintf(stderr, "  number of minted coins mismatch at height %d\n", block->nHeight);
                    return false;
                }
            }

            if (block != coinGroup.second.lastBlock)
                block = (*chain)[block->nHeight+1];
            else
                break;
        }

        fprintf(stderr, "  verified ok\n");
    }

    return true;
}

set<CBlockIndex *> CZerocoinState::RecalculateAccumulators(CChain *chain) {
    set<CBlockIndex *> changes;

    BOOST_FOREACH(const PAIRTYPE(PAIRTYPE(int,int), CoinGroupInfo) &coinGroup, coinGroups) {
        // Skip non-modulusv2 groups
        if (!IsZerocoinTxV2((libzerocoin::CoinDenomination)coinGroup.first.first, Params().GetConsensus(), coinGroup.first.second))
            continue;

        libzerocoin::Accumulator acc(&ZCParamsV2->accumulatorParams, (libzerocoin::CoinDenomination)coinGroup.first.first);

        // Try to calculate accumulator for the first batch of mints. If it doesn't match we need to recalculate the rest of it
        CBlockIndex *block = coinGroup.second.firstBlock;
        for (;;) {
            if (block->accumulatorChanges.count(coinGroup.first) > 0) {
                BOOST_FOREACH(const CBigNum &pubCoin, block->mintedPubCoins[coinGroup.first]) {
                    acc += libzerocoin::PublicCoin(ZCParamsV2, pubCoin, (libzerocoin::CoinDenomination)coinGroup.first.first);
                }

                // First block case is special: do the check
                if (block == coinGroup.second.firstBlock) {
                    if (acc.getValue() != block->accumulatorChanges[coinGroup.first].first)
                        // recalculation is needed
                        LogPrintf("ZerocoinState: accumulator recalculation for denomination=%d, id=%d\n", coinGroup.first.first, coinGroup.first.second);
                    else
                        // everything's ok
                        break;
                }

                block->accumulatorChanges[coinGroup.first] = make_pair(acc.getValue(), (int)block->mintedPubCoins[coinGroup.first].size());
                changes.insert(block);
            }

            if (block != coinGroup.second.lastBlock)
                block = (*chain)[block->nHeight+1];
            else
                break;
        }
    }

    return changes;
}

bool CZerocoinState::AddSpendToMempool(const vector<CBigNum> &coinSerials, uint256 txHash) {
    BOOST_FOREACH(CBigNum coinSerial, coinSerials){
        if (IsUsedCoinSerial(coinSerial) || mempoolCoinSerials.count(coinSerial))
            return false;

        mempoolCoinSerials[coinSerial] = txHash;
    }

    return true;
}

bool CZerocoinState::AddSpendToMempool(const CBigNum &coinSerial, uint256 txHash) {
    if (IsUsedCoinSerial(coinSerial) || mempoolCoinSerials.count(coinSerial))
        return false;

    mempoolCoinSerials[coinSerial] = txHash;
    return true;
}

void CZerocoinState::RemoveSpendFromMempool(const CBigNum &coinSerial) {
    mempoolCoinSerials.erase(coinSerial);
}

uint256 CZerocoinState::GetMempoolConflictingTxHash(const CBigNum &coinSerial) {
    if (mempoolCoinSerials.count(coinSerial) == 0)
        return uint256();

    return mempoolCoinSerials[coinSerial];
}

bool CZerocoinState::CanAddSpendToMempool(const CBigNum &coinSerial) {
    return !IsUsedCoinSerial(coinSerial) && mempoolCoinSerials.count(coinSerial) == 0;
}

extern const char *sigmaRemintBlacklist[];
std::unordered_set<CBigNum,CZerocoinState::CBigNumHash> CZerocoinState::sigmaRemintBlacklistSet;

bool CZerocoinState::IsPublicCoinValueBlacklisted(const CBigNum &value) {
    static bool blackListLoaded = false;

    // Check against black list
    if (!blackListLoaded) {
        AssertLockHeld(cs_main);
        // Initial build of the black list. Thread-safe as we are protected by cs_main
        for (const char **blEntry = sigmaRemintBlacklist; *blEntry; blEntry++) {
            CBigNum bn;
            bn.SetHex(*blEntry);
            sigmaRemintBlacklistSet.insert(bn);
        }
    }

    return sigmaRemintBlacklistSet.count(value) > 0;
}

void CZerocoinState::BlacklistPublicCoinValue(const CBigNum &value) {
    sigmaRemintBlacklistSet.insert(value);
}

void CZerocoinState::Reset() {
    coinGroups.clear();
    usedCoinSerials.clear();
    mintedPubCoins.clear();
    latestCoinIds.clear();
    mempoolCoinSerials.clear();
}

CZerocoinState *CZerocoinState::GetZerocoinState() {
    return &zerocoinState;
}
