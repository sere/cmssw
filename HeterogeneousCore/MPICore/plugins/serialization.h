#ifndef HeterogeneousCore_MPICore_plugins_serialization_h
#define HeterogeneousCore_MPICore_plugins_serialization_h

#include <memory>
#include <utility>

#include <TClass.h>
#include <TBufferFile.h>

// serialise a Wrapper<T> into a char[] via a TBufferFile
inline
std::pair<std::unique_ptr<char[]>, size_t> serialize(edm::WrapperBase const& wrapper)
{
  // TODO
  // construct the buffer with an initial size based on the wrapped class size ?
  // take into account any offset to/from the edm::WrapperBase base class ?
  TBufferFile buffer(TBuffer::kWrite);
  buffer.WriteObjectAny(& wrapper, TClass::GetClass(wrapper.wrappedTypeInfo()));

  size_t size = buffer.Length();
  std::unique_ptr<char[]> data(buffer.Buffer());
  buffer.DetachBuffer();

  return std::make_pair(std::move(data), size);
}

// deserialise a Wrapper<T> from a char[] via a TBufferFile and store it in a unique_ptr<WrapperBase>
inline
std::unique_ptr<edm::WrapperBase> deserialize(char* data, size_t size)
{
  // adopt the memory in a new ROOT buffer
  TBufferFile buffer(TBuffer::kRead, size, data, false);

  // TODO try different versions:
  //std::unique_ptr<edm::WrapperBase> wrapper(reinterpret_cast<edm::WrapperBase *>(buffer.ReadObjectAny(edmWrapperBaseClass)));     // does not work ?
  //std::unique_ptr<edm::WrapperBase> wrapper(reinterpret_cast<edm::WrapperBase *>(buffer.ReadObjectAny(nullptr)));                 // works but maybe not always ?
  //std::unique_ptr<edm::WrapperBase> wrapper(reinterpret_cast<edm::WrapperBase *>(buffer.ReadObjectAny(rootType)));                // not useful ?
  /*
  static TClass const* edmWrapperBaseClass = TClass::GetClass(typeid(edm::WrapperBase));
  TClass * rootType = wrappedType.getClass();
  int offset = rootType->GetBaseClassOffset(edmWrapperBaseClass);
  std::unique_ptr<edm::WrapperBase> wrapper(reinterpret_cast<edm::WrapperBase *>(reinterpret_cast<char *>(buffer.ReadObjectAny(rootType)) + offset));
  */
  std::unique_ptr<edm::WrapperBase> wrapper(reinterpret_cast<edm::WrapperBase *>(buffer.ReadObjectAny(nullptr)));
  return std::move(wrapper);
}

#endif // HeterogeneousCore_MPICore_plugins_serialization_h
