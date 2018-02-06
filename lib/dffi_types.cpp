// Copyright 2018 Adrien Guinet <adrien@guinet.me>
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

#include <dffi/dffi.h>
#include <dffi/composite_type.h>
#include <dffi/casting.h>
#include "dffi_impl.h"

using namespace dffi;

Type::Type(details::DFFIImpl& Dffi, TypeKind K):
  Kind_(K),
  Dffi_(Dffi)
{ }

BasicType::BasicType(details::DFFIImpl& Dffi, BasicKind BKind):
  Type(Dffi, TY_Basic),
  BKind_(BKind)
{ }

unsigned BasicType::getAlign() const
{
  return BasicType::getSize();
}

uint64_t BasicType::getSize() const
{
  switch (BKind_) {
    case Bool:
      return sizeof(bool);
    case Char:
      return sizeof(char);
    case Int8:
      return 1;
    case Int16:
      return 2;
    case Int32:
      return 4;
    case Int64:
      return 8;
    case Int128:
      return 16;
    case UInt8:
      return 1;
    case UInt16:
      return 2;
    case UInt32:
      return 4;
    case UInt64:
      return 8;
    case UInt128:
      return 16;
    case Float32:
      return 4;
    case Float64:
      return 8;
    case Float128:
      return 16;
    case ComplexFloat32:
      return 8;
    case ComplexFloat64:
      return 16;
    case ComplexFloat128:
      return 32;
  }
}

FunctionType::FunctionType(details::DFFIImpl& Dffi, QualType RetTy, ParamsVecTy ParamsTy, CallingConv CC):
  Type(Dffi, TY_Function),
  RetTy_(RetTy),
  ParamsTy_(std::move(ParamsTy))
{
  Flags_.D.CC = CC;
  Flags_.D.VarArgs = 0;
}

NativeFunc FunctionType::getFunction(void* Ptr) const
{
  return getDFFI().getFunction(this, Ptr);
}

PointerType::PointerType(details::DFFIImpl& Dffi, QualType Pointee):
  Type(Dffi, TY_Pointer),
  Pointee_(Pointee)
{ }

PointerType const* PointerType::get(QualType Ty)
{
  return Ty->getDFFI().getPointerType(Ty);
}

BasicType const* EnumType::getBasicType() const
{
  return getDFFI().getBasicType(BasicType::getKind<IntType>());
}

ArrayType::ArrayType(details::DFFIImpl& Dffi, QualType Ty, uint64_t NElements):
  Type(Dffi, TY_Array),
  Ty_(Ty),
  NElements_(NElements)
{ }

CompositeField::CompositeField(const char* Name, Type const* Ty, unsigned Offset):
  Name_(Name),
  Ty_(Ty),
  Offset_(Offset)
{ }

CanOpaqueType::CanOpaqueType(details::DFFIImpl& Dffi, TypeKind Ty):
  Type(Dffi, Ty),
  IsOpaque_(true)
{
  assert(Ty > TY_CanOpaqueType && Ty < TY_CanOpaqueTypeEnd && "invalid mayopaque type");
}

CompositeType::CompositeType(details::DFFIImpl& Dffi, TypeKind Ty):
  CanOpaqueType(Dffi, Ty),
  Size_(0),
  Align_(0)
{
  assert(Ty > TY_Composite && Ty < TY_CompositeEnd && "invalid composite type");
}

CompositeField const* CompositeType::getField(const char* Name) const
{
  auto It = FieldsMap_.find(Name);
  if (It == FieldsMap_.end()) {
    return nullptr;
  }
  return It->second;
}

void CompositeType::setBody(std::vector<CompositeField>&& Fields, uint64_t Size, unsigned Align) {
  assert(isOpaque() && "composite type isn't opaque!");
  setAsDefined();
  Fields_ = std::move(Fields);
  Size_ = Size;
  Align_ = Align;
  for (CompositeField const& F: Fields_) {
    FieldsMap_[F.getName()] = &F;
  }
}

void UnionType::setBody(std::vector<CompositeField>&& Fields, uint64_t Size, unsigned Align) {
  CompositeType::setBody(std::move(Fields), Size, Align);
#ifndef NDEBUG
  for (auto const& F: Fields_) {
    assert(F.getOffset() == 0 && "offset of an union field must be 0!");
  }
#endif
}

void EnumType::setBody(Fields&& Fields) {
  setAsDefined();
  Fields_ = std::move(Fields);
}
