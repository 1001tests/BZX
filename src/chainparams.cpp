// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"
#include "consensus/consensus.h"
#include "priv_params.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "bitcoin_bignum/bignum.h"
#include "blacklists.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"
#include "arith_uint256.h"


static CBlock CreateGenesisBlock(const char *pszTimestamp, const CScript &genesisOutputScript, uint32_t nTime, uint32_t nNonce,
        uint32_t nBits, int32_t nVersion, const CAmount &genesisReward,
        std::vector<unsigned char> extraNonce) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 504365040 << CBigNum(4).getvch() << std::vector < unsigned char >
    ((const unsigned char *) pszTimestamp, (const unsigned char *) pszTimestamp + strlen(pszTimestamp)) << extraNonce;
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount &genesisReward,
                   std::vector<unsigned char> extraNonce) {
    //btzc: BZX timestamp
    const char *pszTimestamp = "Times 2014/10/31 Maine Judge Says Nurse Must Follow Ebola Quarantine for Now";
    const CScript genesisOutputScript = CScript();
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward,
                              extraNonce);
}

// this one is for testing only
static Consensus::LLMQParams llmq5_60 = {
        .type = Consensus::LLMQ_5_60,
        .name = "llmq_5_60",
        .size = 5,
        .minSize = 3,
        .threshold = 3,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 8,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
};

// to use on testnet
static Consensus::LLMQParams llmq10_70 = {
        .type = Consensus::LLMQ_10_70,
        .name = "llmq_10_70",
        .size = 10,
        .minSize = 8,
        .threshold = 7,

        .dkgInterval = 24, // one DKG per hour
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 18,
        .dkgBadVotesThreshold = 8,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
};

static Consensus::LLMQParams llmq50_60 = {
        .type = Consensus::LLMQ_50_60,
        .name = "llmq_50_60",
        .size = 50,
        .minSize = 40,
        .threshold = 30,

        .dkgInterval = 18, // one DKG per 90 minutes
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 16,
        .dkgBadVotesThreshold = 40,

        .signingActiveQuorumCount = 16, // a full day worth of LLMQs

        .keepOldConnections = 17,
};

static Consensus::LLMQParams llmq400_60 = {
        .type = Consensus::LLMQ_400_60,
        .name = "llmq_400_60",
        .size = 400,
        .minSize = 300,
        .threshold = 240,

        .dkgInterval = 12 * 12, // one DKG every 12 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 28,
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs

        .keepOldConnections = 5,
};

// Used for deployment and min-proto-version signalling, so it needs a higher threshold
static Consensus::LLMQParams llmq400_85 = {
        .type = Consensus::LLMQ_400_85,
        .name = "llmq_400_85",
        .size = 400,
        .minSize = 350,
        .threshold = 340,

        .dkgInterval = 12 * 24, // one DKG every 24 hours
        .dkgPhaseBlocks = 4,
        .dkgMiningWindowStart = 20, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 48, // give it a larger mining window to make sure it is mined
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs

        .keepOldConnections = 5,
};


/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";

        consensus.chainType = Consensus::chainMain;

        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP65Height = INT_MAX;
        consensus.BIP66Height = INT_MAX;
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 60 * 60; // 60 minutes between retargets
        consensus.nPowTargetSpacing = 10 * 60; // 10 minute blocks
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1475020800; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("50aff78270725ec253a722ec18069deb233f2e57eb7d64479f027141619cdda4 "); //184200

        // evo znodes
        consensus.DIP0003Height = 278300; // Approximately June 22 2020, 12:00 UTC
        consensus.DIP0003EnforcementHeight = 284400; // Approximately July 13 2020, 12:00 UTC
        consensus.DIP0003EnforcementHash = uint256S("0x8b8d7c05bb2d75f8c5e076cb6c10ef464e94ddcda2744740db03aeda2d6cc006");
        consensus.DIP0008Height = 341100; // Approximately Jan 28 2021, 11:00 UTC
        consensus.nEvoZnodeMinimumConfirmations = 15;

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.nLLMQPowTargetSpacing = 5*60;
        consensus.llmqChainLocks = Consensus::LLMQ_400_60;
        consensus.llmqForInstantSend = Consensus::LLMQ_50_60;

        consensus.nDisableZerocoinStartBlock = 157000;

        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
       `  * a large 32-bit integer with any alignment.
         */
        //btzc: update BZX pchMessage
        pchMessageStart[0] = 0xe3;
        pchMessageStart[1] = 0xd9;
        pchMessageStart[2] = 0xfe;
        pchMessageStart[3] = 0xf1;
        nDefaultPort = 8168;
        nPruneAfterHeight = 100000;
        /**
         * btzc: BZX init genesis block
         * nBits = 0x1e0ffff0
         * nTime = 1414776286
         * nNonce = 142392
         * genesisReward = 0 * COIN
         * nVersion = 2
         * extraNonce
         */
        std::vector<unsigned char> extraNonce(4);
        extraNonce[0] = 0x82;
        extraNonce[1] = 0x3f;
        extraNonce[2] = 0x00;
        extraNonce[3] = 0x00;
        genesis = CreateGenesisBlock(ZC_GENESIS_BLOCK_TIME, 142392, 0x1e0ffff0, 2, 0 * COIN, extraNonce);
        const std::string s = genesis.GetHash().ToString();
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x4381deb85b1b2c9843c222944b616d997516dcbd6a964e1eaf0def0830695233"));
        assert(genesis.hashMerkleRoot == uint256S("0x365d2aa75d061370c9aefdabac3985716b1e3b4bb7c4af4ed54f25e5aaa42783"));
        vSeeds.push_back(CDNSSeedData("amsterdam.BZX.org", "amsterdam.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("australia.BZX.org", "australia.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("chicago.BZX.org", "chicago.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("london.BZX.org", "london.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("frankfurt.BZX.org", "frankfurt.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("newjersey.BZX.org", "newjersey.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("sanfrancisco.BZX.org", "sanfrancisco.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("tokyo.BZX.org", "tokyo.BZX.org", false));
        vSeeds.push_back(CDNSSeedData("singapore.BZX.org", "singapore.BZX.org", false));
        // Note that of those with the service bits flag, most only support a subset of possible options
        base58Prefixes[PUBKEY_ADDRESS] = std::vector < unsigned char > (1, 82);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector < unsigned char > (1, 7);
        base58Prefixes[SECRET_KEY] = std::vector < unsigned char > (1, 210);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container < std::vector < unsigned char > > ();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container < std::vector < unsigned char > > ();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fAllowMultiplePorts = false;

        checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                (0, uint256S("0xf11046292ff76af48b66de6f1a210c09825d2ab4f56975ec507766ebf9c9f443"))
                (14000, uint256S("0xeab9b7e451284cb75ada7609e0220bee2b4f289fed9d9cf2a9e3aa548b2d38eb"))
                (14271, uint256S("0xf15088099a30f98e85a09789880f74cadca42f725c0cc1666484865539d2f335"))
                (20580, uint256S("0x591b00ac1ba7d30b9f440efc467072400805a900e92f04f272e6f70cb55ab026"))
                (121378, uint256S("0xa7d9a56dd2986442b5c10ad036eb4e6555eaa8d9f6645c7b9620597792a153ac"))
                (341100, uint256S("0x1ca6cbd9f6e13db8e0e1db0b77f8b1a037b01c69558214bc1ae2ce1f81da4890"))
        };

        chainTxData = ChainTxData{
                1545712287, // * UNIX timestamp of last checkpoint block
                933513,     // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
                0.014       // * estimated number of transactions per second after checkpoint
        };

        // Sigma related values.
        consensus.nSigmaStartBlock = 50000;
        consensus.nSigmaPaddingBlock = 50001;
        consensus.nLelantusStartBlock = 50002;
        consensus.nMaxSigmaInputPerTransaction = 35;
        consensus.nMaxValueSigmaSpendPerTransaction = (500 * COIN);
        consensus.nMaxLelantusInputPerBlock = 100;
        consensus.nMaxValueLelantusSpendPerBlock = (5101 * COIN);
        consensus.nMaxLelantusInputPerTransaction = 50;
        consensus.nMaxValueLelantusSpendPerTransaction = (5101 * COIN);
        consensus.nMaxValueLelantusMint = (5001 * COIN);

        for (const auto& str : lelantus::lelantus_blacklist) {
            GroupElement coin;
            coin.deserialize(ParseHex(str).data());
            consensus.lelantusBlacklist.insert(coin);
        }

        consensus.evoSporkKeyID = "a78fERshquPsTv2TuKMSsxTeKom56uBwLP";
        consensus.nEvoSporkStartBlock = INT_MAX;
        consensus.nEvoSporkStopBlock = INT_MAX;

        // reorg
        consensus.nMaxReorgDepth = 5;
        consensus.nMaxReorgDepthEnforcementBlock = 338000;

        // Dandelion related values.
        consensus.nDandelionEmbargoMinimum = DANDELION_EMBARGO_MINIMUM;
        consensus.nDandelionEmbargoAvgAdd = DANDELION_EMBARGO_AVG_ADD;
        consensus.nDandelionMaxDestinations = DANDELION_MAX_DESTINATIONS;
        consensus.nDandelionShuffleInterval = DANDELION_SHUFFLE_INTERVAL;
        consensus.nDandelionFluff = DANDELION_FLUFF;

        // Bip39
        consensus.nMnemonicBlock = 222400;
    }

};

static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
    }
};

static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

