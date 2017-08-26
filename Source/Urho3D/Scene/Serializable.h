//
// Copyright (c) 2008-2017 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Core/Attribute.h"
#include "../Core/Object.h"

#include <cstddef>

namespace Urho3D
{

class Connection;
class Deserializer;
class Serializer;
class XMLElement;
class JSONValue;

struct DirtyBits;
struct NetworkState;
struct ReplicationState;

/// Base class for objects with automatic serialization through attributes.
class URHO3D_API Serializable : public Object
{
    URHO3D_OBJECT(Serializable, Object);

public:
    /// Construct.
    Serializable(Context* context);
    /// Destruct.
    virtual ~Serializable();

    /// Handle attribute write access. Default implementation writes to the variable at offset, or invokes the set accessor.
    virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
    /// Handle attribute read access. Default implementation reads the variable at offset, or invokes the get accessor.
    virtual void OnGetAttribute(const AttributeInfo& attr, Variant& dest) const;
    /// Return attribute descriptions, or null if none defined.
    virtual const Vector<AttributeInfo>* GetAttributes() const;
    /// Return network replication attribute descriptions, or null if none defined.
    virtual const Vector<AttributeInfo>* GetNetworkAttributes() const;
    /// Load from binary data. When setInstanceDefault is set to true, after setting the attribute value, store the value as instance's default value. Return true if successful.
    virtual bool Load(Deserializer& source, bool setInstanceDefault = false);
    /// Save as binary data. Return true if successful.
    virtual bool Save(Serializer& dest) const;
    /// Load from XML data. When setInstanceDefault is set to true, after setting the attribute value, store the value as instance's default value. Return true if successful.
    virtual bool LoadXML(const XMLElement& source, bool setInstanceDefault = false);
    /// Save as XML data. Return true if successful.
    virtual bool SaveXML(XMLElement& dest) const;
    /// Load from JSON data. When setInstanceDefault is set to true, after setting the attribute value, store the value as instance's default value. Return true if successful.
    virtual bool LoadJSON(const JSONValue& source, bool setInstanceDefault = false);
    /// Save as JSON data. Return true if successful.
    virtual bool SaveJSON(JSONValue& dest) const;

    /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
    virtual void ApplyAttributes() { }

    /// Return whether should save default-valued attributes into XML. Default false.
    virtual bool SaveDefaultAttributes() const { return false; }

    /// Mark for attribute check on the next network update.
    virtual void MarkNetworkUpdate() { }

    /// Set attribute by index. Return true if successfully set.
    bool SetAttribute(unsigned index, const Variant& value);
    /// Set attribute by name. Return true if successfully set.
    bool SetAttribute(const String& name, const Variant& value);
    /// Reset all editable attributes to their default values.
    void ResetToDefault();
    /// Remove instance's default values if they are set previously.
    void RemoveInstanceDefault();
    /// Set temporary flag. Temporary objects will not be saved.
    void SetTemporary(bool enable);
    /// Enable interception of an attribute from network updates. Intercepted attributes are sent as events instead of applying directly. This can be used to implement client side prediction.
    void SetInterceptNetworkUpdate(const String& attributeName, bool enable);
    /// Allocate network attribute state.
    void AllocateNetworkState();
    /// Write initial delta network update.
    void WriteInitialDeltaUpdate(Serializer& dest, unsigned char timeStamp);
    /// Write a delta network update according to dirty attribute bits.
    void WriteDeltaUpdate(Serializer& dest, const DirtyBits& attributeBits, unsigned char timeStamp);
    /// Write a latest data network update.
    void WriteLatestDataUpdate(Serializer& dest, unsigned char timeStamp);
    /// Read and apply a network delta update. Return true if attributes were changed.
    bool ReadDeltaUpdate(Deserializer& source);
    /// Read and apply a network latest data update. Return true if attributes were changed.
    bool ReadLatestDataUpdate(Deserializer& source);

    /// Return attribute value by index. Return empty if illegal index.
    Variant GetAttribute(unsigned index) const;
    /// Return attribute value by name. Return empty if not found.
    Variant GetAttribute(const String& name) const;
    /// Return attribute default value by index. Return empty if illegal index.
    Variant GetAttributeDefault(unsigned index) const;
    /// Return attribute default value by name. Return empty if not found.
    Variant GetAttributeDefault(const String& name) const;
    /// Return number of attributes.
    unsigned GetNumAttributes() const;
    /// Return number of network replication attributes.
    unsigned GetNumNetworkAttributes() const;

    /// Return whether is temporary.
    bool IsTemporary() const { return temporary_; }

    /// Return whether an attribute's network updates are being intercepted.
    bool GetInterceptNetworkUpdate(const String& attributeName) const;

    /// Return the network attribute state, if allocated.
    NetworkState* GetNetworkState() const { return networkState_.Get(); }

protected:
    /// Network attribute state.
    UniquePtr<NetworkState> networkState_;

private:
    /// Set instance-level default value. Allocate the internal data structure as necessary.
    void SetInstanceDefault(const String& name, const Variant& defaultValue);
    /// Get instance-level default value.
    Variant GetInstanceDefault(const String& name) const;

    /// Attribute default value at each instance level.
    UniquePtr<VariantMap> instanceDefaultValues_;
    /// Temporary flag.
    bool temporary_;
};

/// Attribute metadata.
namespace AttributeMetadata
{
    /// Names of vector struct fields. StringVector.
    static const StringHash P_VECTOR_STRUCT_ELEMENTS("VectorStructElements");
}

// The following macros need to be used within a class member function such as ClassName::RegisterObject().
// A variable called "context" needs to exist in the current scope and point to a valid Context object.

/// Define an attribute with explicitly specified getter and setter expressions. For internal usage only.
#define URHO3D_ATTRIBUTE_IMPL(name, typeName, enumNames, defaultValue, mode, getter, setter) \
    context->RegisterAttribute<ClassName>(Urho3D::AttributeInfo(Urho3D::GetVariantType<typeName>(), name, enumNames, defaultValue, mode, \
        [](const Serializable& objectRef, Variant& value) { auto& object = static_cast<const ClassName&>(objectRef); getter; }, \
        [](Serializable& objectRef, const Variant& value) { auto& object = static_cast<ClassName&>(objectRef); setter; }))

/// Copy attributes from a base class.
#define URHO3D_COPY_BASE_ATTRIBUTES(sourceClassName) context->CopyBaseAttributes<sourceClassName, ClassName>()
/// Remove attribute by name.
#define URHO3D_REMOVE_ATTRIBUTE(name) context->RemoveAttribute<ClassName>(name)
/// Update the default value of an already registered attribute.
#define URHO3D_UPDATE_ATTRIBUTE_DEFAULT_VALUE(name, defaultValue) context->UpdateAttributeDefaultValue<ClassName>(name, defaultValue)
/// Define an attribute that uses get and set functions.
#define URHO3D_ACCESSOR_ATTRIBUTE(name, getFunction, setFunction, typeName, defaultValue, mode) URHO3D_ATTRIBUTE_IMPL(name, typeName, nullptr, defaultValue, mode, value = object.getFunction(), object.setFunction(value.Get<typeName>()))
/// Define an attribute that uses get and set free functions.
#define URHO3D_ACCESSOR_ATTRIBUTE_FREE(name, getFunction, setFunction, typeName, defaultValue, mode) URHO3D_ATTRIBUTE_IMPL(name, typeName, nullptr, defaultValue, mode, value = getFunction(object), setFunction(object, value.Get<typeName>()))
/// Define an attribute that uses get and set functions, and uses zero-based enum values, which are mapped to names through an array of C string pointers.
#define URHO3D_ENUM_ACCESSOR_ATTRIBUTE(name, getFunction, setFunction, typeName, enumNames, defaultValue, mode) URHO3D_ATTRIBUTE_IMPL(name, int, enumNames, defaultValue, mode, value = static_cast<int>(object.getFunction()), object.setFunction(static_cast<typeName>(value.GetInt())))
/// Define an attribute that uses get and set free functions, and uses zero-based enum values, which are mapped to names through an array of C string pointers.
#define URHO3D_ENUM_ACCESSOR_ATTRIBUTE_FREE(name, getFunction, setFunction, typeName, enumNames, defaultValue, mode) URHO3D_ATTRIBUTE_IMPL(name, int, enumNames, defaultValue, mode, value = static_cast<int>(getFunction(object)), setFunction(object, static_cast<typeName>(value.GetInt())))
/// Define an attribute that is a member of the object.
#define URHO3D_ATTRIBUTE(name, typeName, variable, defaultValue, mode) URHO3D_ATTRIBUTE_IMPL(name, typeName, nullptr, defaultValue, mode, value = object.variable, object.variable = value.Get<typeName>())
/// Define an attribute that is a member of the object, and uses zero-based enum values, which are mapped to names through an array of C string pointers.
#define URHO3D_ENUM_ATTRIBUTE(name, variable, enumNames, defaultValue, mode) URHO3D_ATTRIBUTE_IMPL(name, int, enumNames, defaultValue, mode, value = static_cast<int>(object.variable), object.variable = static_cast<decltype(object.variable)>(value.GetInt()))
/// Deprecated. Use URHO3D_ACCESSOR_ATTRIBUTE instead.
#define URHO3D_MIXED_ACCESSOR_ATTRIBUTE(name, getFunction, setFunction, typeName, defaultValue, mode) URHO3D_ACCESSOR_ATTRIBUTE(name, getFunction, setFunction, typeName, defaultValue, mode)
/// Deprecated. Use URHO3D_ACCESSOR_ATTRIBUTE_FREE instead.
#define URHO3D_MIXED_ACCESSOR_ATTRIBUTE_FREE(name, getFunction, setFunction, typeName, defaultValue, mode) URHO3D_ACCESSOR_ATTRIBUTE_FREE(name, getFunction, setFunction, typeName, defaultValue, mode)

}
