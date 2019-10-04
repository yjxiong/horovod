// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

#ifndef HOROVOD_MXNET_MPI_OPS_H
#define HOROVOD_MXNET_MPI_OPS_H

#include <mxnet/base.h>
#include <mxnet/c_api.h>
#include <mxnet/c_api_error.h>
#include <mxnet/engine.h>
#include <mxnet/ndarray.h>

#include "adapter.h"
#include "tensor_util.h"

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

namespace horovod {
namespace mxnet {

using namespace horovod::common;

typedef ::mxnet::NDArray NDArray;
typedef ::mxnet::Engine::CallbackOnComplete CallbackOnComplete;
typedef Request::RequestType OperationType;
typedef std::shared_ptr<MXTensor<NDArray>> MXTensorSharedPtr;


/*
This class is added to blocking wait the operation in horovod
*/
class HorovodSynchronizer{
public:
  HorovodSynchronizer():cv_(),mtx_(),done_(false){

  }
  void MarkDone();
  bool Wait();
private:
  std::condition_variable cv_;
  std::mutex mtx_;
  bool done_;
};

typedef std::shared_ptr<HorovodSynchronizer> SyncSharedPtr;

struct MpiOpsParam {
  NDArray* input;
  NDArray* output;
  MXTensorSharedPtr cpu_tensor;
  MXTensorSharedPtr output_tensor;
  OperationType op_type;
  std::string op_name;
  int root_rank;
  SyncSharedPtr sync_handle;

  MpiOpsParam(NDArray* input, NDArray* output,
              MXTensorSharedPtr cpu_tensor,
              MXTensorSharedPtr output_tensor,
              const OperationType& op_type, const std::string& op_name,
              int root_rank, SyncSharedPtr sync_handle)
      : input(input),
        output(output),
        cpu_tensor(cpu_tensor),
        output_tensor(output_tensor),
        op_type(op_type),
        op_name(op_name),
        root_rank(root_rank),
        sync_handle(sync_handle) {
  }
};

inline MpiOpsParam* CreateMpiOpsParam(NDArray* input, NDArray* output,
                                      MXTensorSharedPtr cpu_tensor, MXTensorSharedPtr output_tensor,
                                      const OperationType& op_type,
                                      const std::string& op_name,
                                      int root_rank, 
                                      SyncSharedPtr sync_handle) {
  return new MpiOpsParam(input, output, cpu_tensor, output_tensor, op_type, op_name, root_rank, sync_handle);
}

void DeleteMpiOpsParam(void* param) {
  auto ops_param = static_cast<MpiOpsParam*>(param);
  delete ops_param;
}

extern "C" int horovod_mxnet_allreduce_async(NDArray* input, NDArray* output,
                                             const char* name, bool average,
                                             int priority);
extern "C" int horovod_mxnet_allgather_async(NDArray* input, NDArray* output,
                                             const char* name, int priority);
extern "C" int horovod_mxnet_broadcast_async(NDArray* input, NDArray* output,
                                             const char* name, int root_rank,
                                             int priority);


} // namespace mxnet
} // namespace horovod

#endif // HOROVOD_MXNET_MPI_OPS_H
