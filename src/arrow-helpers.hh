#pragma once

#include "arrow/array/builder_nested.h"
#include "arrow/memory_pool.h"
#include "arrow/status.h"
#include <arrow/api.h>

#include <type_traits>
#include <vector>


template<class InnerBuilder>
struct list_builder {
  using InnerType = decltype(std::declval<InnerBuilder>().GetValue(0));

  template<class... Args>
  list_builder(arrow::MemoryPool* pool, InnerType element_type, Args... args)
    : element_builder{std::make_shared<InnerBuilder>(pool, std::forward<Args>(args)...)}
    ,   entry_builder{pool, element_builder, element_type}
  { }

  arrow::Status Append(const std::vector<InnerType>& data) {
    ARROW_RETURN_NOT_OK(  entry_builder .  Append      (    ));
    ARROW_RETURN_NOT_OK(element_builder -> AppendValues(data));
    return arrow::Status::OK();
  }

  arrow::Result<std::shared_ptr<arrow::Array>> Finish();

private:
  arrow::ListBuilder              entry_builder;
  std::shared_ptr<InnerBuilder> element_builder;
};

template<class BUILDER, class...Args>
std::shared_ptr<BUILDER> builder_from_type(arrow::MemoryPool* pool, Args&&... args) {
  if (std::is_same_v<BUILDER, arrow::)
  return std::make_shared<BUILDER>(pool, std::forward<Args>(args)....);
}

struct struct_builder {
  struct_builder(arrow::MemoryPool* pool) : pool{pool} {};

  // .field<arrow::uint32>("name", true)
  // .field<arrow::list  >("name", true, arrow::uint32())
  template<class FIELD, class... Args>
  struct_builder& field(const std::string& name, bool nullable, Args&&... args) {
    auto field = arrow::field(name, FIELD{std::forward<Args>(args)...}, nullable);
    fields.push_back(field);
  }

  template<class BUILDER, class... Args>
  struct_builder& builder(Args&&... args) {
    auto builder = std::make_shared<BUILDER>(pool, std::forward<Args>(args)...);
    builders.push_bash(builder);
  }

  arrow::struct_ type() {
    return {fields};
  }

  arrow::StructBuilder done() {
    return {type(), pool, builders}
  }

private:
  arrow::MemoryPool*                                    pool;
  std::vector<std::shared_ptr<arrow::Field>>          fields;
  std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
}



// template<class... InnerBuilders>
// struct struct_builder {
//   struct_builder(arrow::MemoryPool* pool, StructType struct_type, Args... args)
//     : field_builders{std::make_shared<FieldBuilder>(pool, )}
//     ,   entry_builder{pool, field_builders, struct_type} {}

// private:
//   arrow::StructBuilder2 entry_builder;
// };




namespace arrow {
  // Similar structure to all builders
  // Primitives      args: pool
  // list            args: pool, field_builder , type
  // fixed-size list args: pool, field_builder , type
  // struct          args: pool, field_builders, type
  struct StructBuilder2 : public StructBuilder {
    StructBuilder2( arrow::MemoryPool* pool
                  , std::vector<std::shared_ptr<ArrayBuilder>>& field_builders
                  , const std::shared_ptr<DataType>& type)
    : StructBuilder{type, pool, field_builders}
    { }
  };
} // namespace arrow
