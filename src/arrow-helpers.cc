#include "arrow-helpers.hh"


list_builder::list_builder(arrow::MemoryPool* pool = arrow::default_memory_pool())
:   entry_builder{pool, std::make_shared<InnerBuilder>{pool}, inner_type}
  , element_builer{static_cast<InnerBuilder*>(entry_builder.value_builder())}
{}

list_builder::list_builder(arrow::MemoryPool* pool = arrow::default_memory_pool(), InnerType inner_type)
:   entry_builder{pool, std::make_shared<InnerBuilder>{pool}, inner_type}
  , element_builer{static_cast<InnerBuilder*>(entry_builder.value_builder())}
{}
