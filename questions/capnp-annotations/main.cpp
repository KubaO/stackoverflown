#include <capnp/schema.h>
#include <capnp/schema-loader.h>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <map>
#include <vector>
#include "annotations.capnp.h"

using namespace capnp;

#if 0
class AnnotationsLoader : private kj::DisallowConstCopy {

      std::vector<schema::Annotation::Reader
   };

   using Annotations = std::map<uint64_t, schema::Annotation::Reader>;
   using AnnotationsForTypes = std::map<uint64_t, schema::Annotation>;
public:
   using Id = uint64_t;



private:
   AnnotationsForTypes data;
};
#endif

using AnnotationList = List<schema::Annotation, Kind::STRUCT>::Reader;

template <typename T> class Tag {};

class Annotations {
   SchemaLoader &loader;
  public:
   using Id = uint64_t;
   using Ids = std::vector<Id>;
   using map_type = std::map<Id, schema::Annotation::Reader>;
   map_type map;
   std::map<uint64_t, schema::Node::Annotation> nodes;

   template <typename T>
   Annotations(SchemaLoader &loader, Tag<T>) : loader(loader) {
      loader.loadCompiledTypeAndDependencies<T>();
      add(loader.get(typeId<T>()));
   }

   Ids add(schema::Node::Reader node) {
      loader.load(node);
      return add(node.getAnnotations());
   }

   Ids add(const Type &type) {
      switch (type.which()) {
         case schema::Type::Which::STRUCT:
            return add(type.asStruct());
         case schema::Type::Which::ENUM:
            return add(type.asEnum());
         case schema::Type::Which::INTERFACE:
            return add(type.asInterface());
         case schema::Type::Which::LIST:
            return add(type.asList());
      default:
         return {};
      }
   }

   Ids add(const Schema &schema) {
      auto which = schema.getProto().which();
      switch (which) {
         case schema::Node::Which::STRUCT:
            return add(schema.asStruct());
         case schema::Node::Which::ENUM:
            return add(schema.asEnum());
         case schema::Node::Which::INTERFACE:
            return add(schema.asInterface());
      default:
         return {};
      }
   }

   Ids add(const StructSchema &schema) {
      auto added = add(schema.getProto());
      for (auto field : schema.getFields()) {
         const auto ids = add(field.getType());
         std::copy(ids.begin(), ids.end(), std::back_inserter(added));
      }
      for (auto id : std::as_const(added)) {
         KJ_IF_MAYBE(schema, loader.tryGet(id)) {
            const auto ids = add(*schema);
            std::copy(ids.begin(), ids.end(), std::back_inserter(added));
         } else {
            std::cout << "ignoring schema id " << id << "\n";
         }
       }
      return added;
   }

   Ids add(const EnumSchema &schema) { return add(schema.getProto()); }

   Ids add(const InterfaceSchema &schema) { return add(schema.getProto()); }

   Ids add(const ListSchema &schema) { return add(schema.getElementType()); }

   Ids add(AnnotationList list) {
      Ids added;
      for (auto annotation : list) {
         auto id = annotation.getId();
         added.push_back(id);
         map[id] = annotation;
      }
      return added;
   }

   const map_type &get() const { return map; }
   schema::Annotation::Reader &get(Id id) {
      static schema::Annotation::Reader defaultReader;
      auto it = map.find(id);
      if (it != map.end()) return it->second;
      return defaultReader;
   }
};

int main() {
   SchemaLoader loader;
   auto annotations = Annotations(loader, Tag<Struct>());
   for (auto &a : annotations.get()) {
      auto &annot = a.second;
      std::cout << annot.toString().flatten().cStr() << "\n";
   }
   for (auto schema : loader.getAllLoaded()) {
      std::cout << "loaded " << schema.getShortDisplayName().cStr() << "\n";
   }
}
