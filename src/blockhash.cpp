
// Copyright (c) 2020 Michael Toutonghi
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "solutiondata.h"

[[noreturn]] void new_handler_terminate()
{
    // Rather than throwing std::bad-alloc if allocation fails, terminate
    // immediately to (try to) avoid chain corruption.
    // Since LogPrintf may itself allocate memory, set the handler directly
    // to terminate first.
    std::set_new_handler(std::terminate);
    fputs("Error: Out of memory. Terminating.\n", stderr);

    // The log was successful, terminate now.
    std::terminate();
};

uint256 CBlockHeader::GetVerusV2Hash() const
{
    if (hashPrevBlock.IsNull())
    {
        // always use SHA256D for genesis block
        return SerializeHash(*this);
    }
    else
    {
        if (nVersion == VERUS_V2)
        {
            int solutionVersion = CConstVerusSolutionVector::Version(nSolution);

            // in order for this to work, the PBaaS hash of the pre-header must match the header data
            // otherwise, it cannot clear the canonical data and hash in a chain-independent manner
            int pbaasType = CConstVerusSolutionVector::HasPBaaSHeader(nSolution);
            //bool debugPrint = false;
            //if (pbaasType != 0 && solutionVersion == CActivationHeight::SOLUTION_VERUSV5_1)
            //{
            //    debugPrint = true;
            //    printf("%s: version V5_1 header, pbaasType: %d, CheckNonCanonicalData: %d\n", __func__, pbaasType, CheckNonCanonicalData());
            //}
            if (pbaasType != 0 && CheckNonCanonicalData())
            {
                CBlockHeader bh = CBlockHeader(*this);
                bh.ClearNonCanonicalData();
                //if (debugPrint)
                //{
                //    printf("%s\n", SerializeVerusHashV2b(bh, solutionVersion).GetHex().c_str());
                //    printf("%s\n", SerializeVerusHashV2b(*this, solutionVersion).GetHex().c_str());
                //}
                return SerializeVerusHashV2b(bh, solutionVersion);
            }
            else
            {
                //if (debugPrint)
                //{
                //    printf("%s\n", SerializeVerusHashV2b(*this, solutionVersion).GetHex().c_str());
                //}
                return SerializeVerusHashV2b(*this, solutionVersion);
            }
        }
        else
        {
            return SerializeVerusHash(*this);
        }
    }
}

CPBaaSPreHeader::CPBaaSPreHeader(const CBlockHeader &bh)
{
    hashPrevBlock = bh.hashPrevBlock;
    hashMerkleRoot = bh.hashMerkleRoot;
    hashFinalSaplingRoot = bh.hashFinalSaplingRoot;
    nNonce = bh.nNonce;
    nBits = bh.nBits;
    CPBaaSSolutionDescriptor descr = CConstVerusSolutionVector::GetDescriptor(bh.nSolution);
    if (descr.version >= CConstVerusSolutionVector::activationHeight.ACTIVATE_PBAAS_HEADER)
    {
        hashPrevMMRRoot = descr.hashPrevMMRRoot;
        hashBlockMMRRoot = descr.hashBlockMMRRoot;
    }
}
