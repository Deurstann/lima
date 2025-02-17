// Copyright 2021 CEA LIST
// SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
//
// SPDX-License-Identifier: MIT

#ifndef DEEPLIMA_TAGGING_EIGEN_INFERENCE_IMPL_H
#define DEEPLIMA_TAGGING_EIGEN_INFERENCE_IMPL_H

#include <eigen3/Eigen/Dense>

#include "bilstm.h"
#include "bilstm_and_dense.h"

#include "embd_dict.h"

#include "deeplima/segmentation/impl/char_ngram.h"
#include "birnn_inference_base.h"

namespace deeplima
{
namespace tagging
{
namespace eigen_impl
{
#ifdef WIN32
  #define TAG_EXPORT __declspec(dllexport)
#else
  #define TAG_EXPORT
#endif
template <class M, class V, class T>
class TAG_EXPORT BiRnnEigenInferenceForTagging : public deeplima::eigen_impl::BiRnnInferenceBase<M, V, T>
{
public:
  typedef M Matrix;
  typedef V Vector;
  typedef T Scalar;
  typedef M tensor_t;
  typedef EmbdUInt64FloatHolder dicts_holder_t;
  typedef deeplima::eigen_impl::BiRnnInferenceBase<M, V, T> Parent;

  virtual void load(const std::string& fn)
  {
    convert_from_torch(fn);
  }

  virtual size_t get_precomputed_dim() const
  {
    typename deeplima::eigen_impl::Op_BiLSTM_Dense_ArgMax<M, V, T>::params_t *p_params =
      static_cast<typename deeplima::eigen_impl::Op_BiLSTM_Dense_ArgMax<M, V, T>::params_t*>(Parent::m_params[0]);

    const auto& layer = p_params->bilstm;
    size_t hidden_size = layer.fw.weight_ih.rows() + layer.bw.weight_ih.rows();
    return hidden_size;
  }

  virtual void precompute_inputs(
      const M& inputs,
      M& outputs,
      int64_t input_size
      )
  {
    deeplima::eigen_impl::Op_BiLSTM_Dense_ArgMax<M, V, T> *p_op
        = static_cast<deeplima::eigen_impl::Op_BiLSTM_Dense_ArgMax<M, V, T>*>(Parent::m_ops[0]);

    p_op->precompute_inputs(Parent::m_params[0], inputs, outputs, input_size);
  }

  virtual void predict(
      size_t worker_id,
      const M& inputs,
      int64_t input_begin,
      int64_t input_end,
      int64_t output_begin,
      int64_t output_end,
      std::vector<std::vector<uint8_t>>& output,
      const std::vector<std::string>& outputs_names
      )
  {
    deeplima::eigen_impl::Op_BiLSTM_Dense_ArgMax<M, V, T> *p_op
        = static_cast<deeplima::eigen_impl::Op_BiLSTM_Dense_ArgMax<M, V, T>*>(Parent::m_ops[0]);
    assert(Parent::m_wb.size() > 0);
    assert(worker_id < Parent::m_wb[0].size());
    p_op->execute(Parent::m_wb[0][worker_id],
        inputs, Parent::m_params[0], output,
        input_begin, input_end,
        output_begin, output_end);
  }

  const std::vector<std::vector<std::string>>& get_classes() const
  {
    return m_classes;
  }

  const std::vector<std::string>& get_class_names() const
  {
    return m_class_names;
  }

  inline const std::string& get_embd_fn(size_t idx) const
  {
    return m_embd_fn[idx];
  }

protected:
  std::vector<std::string> m_class_names;
  std::vector<std::vector<std::string>> m_classes;
  std::vector<std::string> m_embd_fn;

  virtual void convert_from_torch(const std::string& fn);
};

typedef BiRnnEigenInferenceForTagging<Eigen::MatrixXf, Eigen::VectorXf, float> BiRnnEigenInferenceForTaggingF;

} // namespace eigen_impl
} // namespace tagging
} // namespace deeplima

#endif
