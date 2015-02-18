/*******************************************************************************
 *
 *  Copyright (c) 2014 Advanced Micro Devices, Inc. (unpublished)
 *
 *  All rights reserved.  This notice is intended as a precaution against
 *  inadvertent publication and does not imply publication or any waiver
 *  of confidentiality.  The year included in the foregoing notice is the
 *  year of creation of the work.
 *
 ******************************************************************************/

#include "hwdebug.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

namespace amd {

class Device;

/*
 ***************************************************************************
 *                  Implementation of GPU Debug Manager class
 ***************************************************************************
 */

//!  Constructor of the debug manager class
HwDebugManager::HwDebugManager(amd::Device* device)
    : context_(NULL)
    , device_(device)
    , preDispatchCallBackFunc_(NULL)
    , postDispatchCallBackFunc_(NULL)
    , preDispatchCallBackArgs_(NULL)
    , postDispatchCallBackArgs_(NULL)
    , paramMemory_(NULL)
    , numParams_(0)
    , aclBinary_(NULL)
    , aqlCodeAddr_(NULL)
    , aqlCodeSize_(0)
    , scratchRingAddr_(NULL)
    , scratchRingSize_(0)
    , isRegistered_(false)
    , dbgMsgBufferReady_(false)
{
    memset(&debugInfo_, 0, sizeof(debugInfo_));

    for (int i = 0; i < kDebugTrapLocationMax; i++) {
        rtTrapInfo_[i] = NULL;
    }
}

HwDebugManager::~HwDebugManager()
{
    if (NULL != paramMemory_) {
        delete[] paramMemory_;
    }
}

//!  Setup the call back function pointer
void
HwDebugManager::setCallBackFunctions(cl_PreDispatchCallBackFunctionAMD preDispatchFunction,
                                     cl_PostDispatchCallBackFunctionAMD postDispatchFunction)
{
    preDispatchCallBackFunc_ = preDispatchFunction;
    postDispatchCallBackFunc_ = postDispatchFunction;
}

//!  Setup the call back argument pointers
void
HwDebugManager::setCallBackArguments(void* preDispatchArgs, void* postDispatchArgs)
{
    preDispatchCallBackArgs_ = preDispatchArgs;
    postDispatchCallBackArgs_ = postDispatchArgs;
}

//!  Get dispatch debug info
void
HwDebugManager::getDispatchDebugInfo(void* debugInfo) const
{
    memcpy(debugInfo, (void*) &debugInfo_, sizeof(DispatchDebugInfo));
}


//!  Set the kernel code address and its size
void
HwDebugManager::setKernelCodeInfo(address aqlCodeAddr, uint32_t aqlCodeSize)
{
    aqlCodeAddr_ = aqlCodeAddr;
    aqlCodeSize_ = aqlCodeSize;
}

//!  Get the scratch ring
void
HwDebugManager::setScratchRing(address scratchRingAddr, uint32_t scratchRingSize)
{
    scratchRingAddr_ = scratchRingAddr;
    scratchRingSize_ = scratchRingSize;
}

//!  Map the shader (AQL code) for host access
void
HwDebugManager::mapKernelCode(uint64_t* aqlCodeAddr, uint32_t* aqlCodeSize) const
{
    *aqlCodeAddr = reinterpret_cast<uint64_t>(aqlCodeAddr_);
    *aqlCodeSize = aqlCodeSize_;
}

//!  Map the scratch ring for host access
void
HwDebugManager::mapScratchRing(uint64_t* scratchRingAddr, uint32_t* scratchRingSize) const
{
    *scratchRingAddr = reinterpret_cast<uint64_t>(scratchRingAddr_);
    *scratchRingSize = scratchRingSize_;
}

void
HwDebugManager::setExceptionPolicy(void* exceptionPolicy)
{
    memcpy(&excpPolicy_, exceptionPolicy, sizeof(cl_dbg_exception_policy_amd));
}

void
HwDebugManager::getExceptionPolicy(void* exceptionPolicy) const
{
    memcpy(exceptionPolicy, &excpPolicy_, sizeof(cl_dbg_exception_policy_amd));
}

void
HwDebugManager::setKernelExecutionMode(void* mode)
{
    cl_dbg_kernel_exec_mode_amd* execMode = reinterpret_cast<cl_dbg_kernel_exec_mode_amd*>(mode);
    execMode_.ui32All = execMode->ui32All;
}


void
HwDebugManager::getKernelExecutionMode(void* mode) const
{
    cl_dbg_kernel_exec_mode_amd* execMode = reinterpret_cast<cl_dbg_kernel_exec_mode_amd*>(mode);
    execMode->ui32All = execMode_.ui32All;
}

void
HwDebugManager::setAclBinary(void* aclBinary)
{
    aclBinary_ = aclBinary;
}

void
HwDebugManager::allocParamMemList(uint32_t numParams)
{
    if (NULL != paramMemory_) {
        delete [] paramMemory_;
    }

    numParams_ = numParams;
    paramMemory_ = new amd::Memory*[numParams];
}

cl_mem
HwDebugManager::getKernelParamMem(uint32_t paramIdx) const
{
    assert((paramIdx < numParams_) && "Invalid kernel parameter index too big");

    return as_cl(paramMemory_[paramIdx]);
}

void
HwDebugManager::assignKernelParamMem(uint32_t paramIdx, amd::Memory* mem)
{
    assert((paramIdx < numParams_) && "Invalid kernel parameter index too big");

    paramMemory_[paramIdx] = mem;
}

void
HwDebugManager::installTrap(cl_dbg_trap_type_amd  trapType,
                            amd::Memory*          trapHandler,
                            amd::Memory*          trapBuffer)
{
    rtTrapInfo_[trapType<<2] = trapHandler;
    rtTrapInfo_[(trapType<<2)+1] = trapBuffer;
}


} // namespace amd
