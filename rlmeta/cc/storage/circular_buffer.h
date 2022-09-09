// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <torch/extension.h>
#include <torch/torch.h>

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "rlmeta/cc/storage/circular_buffer_base.h"

namespace py = pybind11;

namespace rlmeta {

class CircularBuffer : public CircularBufferBase {
 public:
  explicit CircularBuffer(int64_t capacity) : CircularBufferBase(capacity) {
    data_.reserve(capacity_);
  }

  int64_t Size() const override { return data_.size(); }

  int64_t next_key() const { return next_key_; }

  void Clear() override {
    CircularBufferBase::Clear();
    data_.clear();
    data_.reserve(capacity_);
    key_to_index_.clear();
    next_key_ = 0;
  }

  py::object At(int64_t key) const override {
    const auto it = key_to_index_.find(key);
    return it == key_to_index_.end() ? py::none() : data_.at(it->second).second;
  }

  py::object At(const py::array_t<int64_t>& keys) const override {
    assert(index.ndim() == 1);
    return BatchedAtImpl(keys.size(), keys.data());
  }

  py::object At(const torch::Tensor& keys) const override {
    assert(keys.dtype() == torch::kInt64);
    assert(keys.dim() == 1);
    const torch::Tensor keys_contiguous = keys.contiguous();
    return BatchedAtImpl(keys_contiguous.numel(),
                         keys_contiguous.data_ptr<int64_t>());
  }

  std::pair<std::int64_t, std::optional<int64_t>> Append(
      const py::object& o) override;

  std::pair<py::array_t<int64_t>, py::array_t<int64_t>> Extend(
      const py::tuple& src) override {
    return ExtendImpl(src);
  }

  std::pair<py::array_t<int64_t>, py::array_t<int64_t>> Extend(
      const py::list& src) override {
    return ExtendImpl(src);
  }

  py::tuple DumpData() const;

  void LoadData(const py::tuple& src, int64_t cursor, int64_t next_key);

 protected:
  py::tuple BatchedAtImpl(int64_t n, const int64_t* keys) const;

  template <class Sequence>
  std::pair<py::array_t<int64_t>, py::array_t<int64_t>> ExtendImpl(
      const Sequence& src);

  std::vector<std::pair<int64_t, py::object>> data_;
  std::unordered_map<int64_t, int64_t> key_to_index_;
  int64_t next_key_ = 0;
};

void DefineCircularBuffer(py::module& m);

}  // namespace rlmeta
