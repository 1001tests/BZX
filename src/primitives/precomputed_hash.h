// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2016-2017 The Firo Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

static const char *precomputedHash[20501] = {
        "",
};

// We rarely reference PoW hash of early mainnet blocks, no need to convert it to uint256 for easy access
uint256 GetPrecomputedBlockPoWHash(int nHeight) {

        return uint256();
}
