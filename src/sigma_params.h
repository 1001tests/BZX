#ifndef ZEROCOIN_PARAMS_H
#define ZEROCOIN_PARAMS_H

/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 1000;   // 0.00001 BZX mininput

// Block after which lelantus mints are activated.
#define ZC_LELANTUS_STARTING_BLOCK         999999//xxxx

// limit of coins number per id in Lelantus
#define ZC_LELANTUS_MAX_MINT_NUM    65000
#define ZC_LELANTUS_SET_START_SIZE  16000

// Version of the block index entry that introduces Lelantus protocol
#define LELANTUS_PROTOCOL_ENABLEMENT_VERSION	999999//xxxx
// Version of the block index enty that introduces evo sporks
#define EVOSPORK_MIN_VERSION                999999

// number of mint confirmations needed to spend coin
#define ZC_MINT_CONFIRMATIONS               2

// Value of lelantus spends allowed per block
#define ZC_LELANTUS_VALUE_SPEND_LIMIT_PER_BLOCK  (5100 * COIN)

// Amount of lelantus spends allowed per block
#define ZC_LELANTUS_INPUT_LIMIT_PER_BLOCK         100

// Value of lelantus spends allowed per transaction
#define ZC_LELANTUS_VALUE_SPEND_LIMIT_PER_TRANSACTION     (5001 * COIN)

// Amount of lelantus spends allowed per transaction
#define ZC_LELANTUS_INPUT_LIMIT_PER_TRANSACTION            50

// Maximum amount of lelantus mint
#define ZC_LELANTUS_MAX_MINT            (5001 * COIN)

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

// Versions of mint/spend transactions
#define LELANTUS_TX_VERSION_4_5               45

#endif
