#pragma once

#include <arrow/api.h>

template<class InnerBuilder>
struct list_builder {
  using InnerType = decltype(std::declval<InnerBuilder>().GetValue(0));

  list_builder(arrow::MemoryPool* pool = arrow::default_memory_pool());
  list_builder(arrow::MemoryPool* pool = arrow::default_memory_pool(), InnerType inner_type);

  arrow::Status Append(const std::vector<InnerType>& data);
  arrow::Result<std::shared_ptr<arrow::Array> Finish();

private:
  arrow::ListBuider     entry_builder;
  InnerBuilder element_builder;
};

template<class... InnerBuilders>
struct struct_builder {
  struct_builder(arrow::MemoryPool* pool = arrow::default_memory_pool());

private:
  StructBuilder entry_builder;
};
