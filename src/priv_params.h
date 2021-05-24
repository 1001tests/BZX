#ifndef BZX_PARAMS_H
#define BZX_PARAMS_H



// limit of coins number per id in Lelantus
#define ZC_LELANTUS_MAX_MINT_NUM    65000
#define ZC_LELANTUS_SET_START_SIZE  16000
// Version of the block index entry that introduces Lelantus protocol
#define LELANTUS_PROTOCOL_ENABLEMENT_VERSION	140100
// Version of the block index enty that introduces evo sporks
#define EVOSPORK_MIN_VERSION                140200

// number of mint confirmations needed to spend coin
#define ZC_MINT_CONFIRMATIONS               2


/** Maximum number of outbound peers designated as Dandelion destinations */
#define DANDELION_MAX_DESTINATIONS 2

/** Expected time between Dandelion routing shuffles (in seconds). */
#define DANDELION_SHUFFLE_INTERVAL 600

/** The minimum amount of time a Dandelion transaction is embargoed (seconds) */
#define DANDELION_EMBARGO_MINIMUM 10
#define DANDELION_TESTNET_EMBARGO_MINIMUM 1

/** The average additional embargo time beyond the minimum amount (seconds) */
#define DANDELION_EMBARGO_AVG_ADD 20
#define DANDELION_TESTNET_EMBARGO_AVG_ADD 1

/** Probability (percentage) that a Dandelion transaction enters fluff phase */
#define DANDELION_FLUFF 10

// Versions of zerocoin mint/spend transactions
#define ZEROCOIN_TX_VERSION_3               30
#define ZEROCOIN_TX_VERSION_3_1             31
#define LELANTUS_TX_VERSION_4               40
#define SIGMA_TO_LELANTUS_JOINSPLIT         41
#define LELANTUS_TX_VERSION_4_5             45
#define SIGMA_TO_LELANTUS_JOINSPLIT_FIXED   46
#define ZEROCOIN_PUBLICKEY_TO_SERIALNUMBER  "PUBLICKEY_TO_SERIALNUMBER"

#endif
