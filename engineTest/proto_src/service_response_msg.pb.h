// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: service_response_msg.proto

#ifndef PROTOBUF_service_5fresponse_5fmsg_2eproto__INCLUDED
#define PROTOBUF_service_5fresponse_5fmsg_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace test {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_service_5fresponse_5fmsg_2eproto();
void protobuf_AssignDesc_service_5fresponse_5fmsg_2eproto();
void protobuf_ShutdownFile_service_5fresponse_5fmsg_2eproto();

class service_response_msg;

// ===================================================================

class service_response_msg : public ::google::protobuf::Message {
 public:
  service_response_msg();
  virtual ~service_response_msg();

  service_response_msg(const service_response_msg& from);

  inline service_response_msg& operator=(const service_response_msg& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const service_response_msg& default_instance();

  void Swap(service_response_msg* other);

  // implements Message ----------------------------------------------

  service_response_msg* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const service_response_msg& from);
  void MergeFrom(const service_response_msg& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string name = 4;
  inline bool has_name() const;
  inline void clear_name();
  static const int kNameFieldNumber = 4;
  inline const ::std::string& name() const;
  inline void set_name(const ::std::string& value);
  inline void set_name(const char* value);
  inline void set_name(const char* value, size_t size);
  inline ::std::string* mutable_name();
  inline ::std::string* release_name();
  inline void set_allocated_name(::std::string* name);

  // @@protoc_insertion_point(class_scope:test.service_response_msg)
 private:
  inline void set_has_name();
  inline void clear_has_name();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::std::string* name_;
  friend void  protobuf_AddDesc_service_5fresponse_5fmsg_2eproto();
  friend void protobuf_AssignDesc_service_5fresponse_5fmsg_2eproto();
  friend void protobuf_ShutdownFile_service_5fresponse_5fmsg_2eproto();

  void InitAsDefaultInstance();
  static service_response_msg* default_instance_;
};
// ===================================================================


// ===================================================================

// service_response_msg

// optional string name = 4;
inline bool service_response_msg::has_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void service_response_msg::set_has_name() {
  _has_bits_[0] |= 0x00000001u;
}
inline void service_response_msg::clear_has_name() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void service_response_msg::clear_name() {
  if (name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_->clear();
  }
  clear_has_name();
}
inline const ::std::string& service_response_msg::name() const {
  // @@protoc_insertion_point(field_get:test.service_response_msg.name)
  return *name_;
}
inline void service_response_msg::set_name(const ::std::string& value) {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  name_->assign(value);
  // @@protoc_insertion_point(field_set:test.service_response_msg.name)
}
inline void service_response_msg::set_name(const char* value) {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  name_->assign(value);
  // @@protoc_insertion_point(field_set_char:test.service_response_msg.name)
}
inline void service_response_msg::set_name(const char* value, size_t size) {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  name_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:test.service_response_msg.name)
}
inline ::std::string* service_response_msg::mutable_name() {
  set_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    name_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:test.service_response_msg.name)
  return name_;
}
inline ::std::string* service_response_msg::release_name() {
  clear_has_name();
  if (name_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = name_;
    name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void service_response_msg::set_allocated_name(::std::string* name) {
  if (name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete name_;
  }
  if (name) {
    set_has_name();
    name_ = name;
  } else {
    clear_has_name();
    name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:test.service_response_msg.name)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace test

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_service_5fresponse_5fmsg_2eproto__INCLUDED
