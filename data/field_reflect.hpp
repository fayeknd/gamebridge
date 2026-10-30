#pragma once
#include <string_view>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace Engine {
    // no runtime overhead with marcos!
    #define PUBLIC [[maybe_unused]]

    // the reflectable field. stores information about a reflected field. 
    struct ReflectableField {
        std::string_view m_fieldName;
        size_t m_fieldOffset;
        std::string_view m_fieldType;
    };

    namespace Reflection {
      
        template <typename T>
        static const std::vector<ReflectableField>& getComponentFields();

        template <typename T>
        static const std::string_view& getComponentTypename();

        template <typename T, typename ComponentType>
        static const T* getFieldPtr(const ComponentType& component, const ReflectableField& field) {
            // get byte address of component
            auto* byteAddress = reinterpret_cast<uint8_t*>(component);
            
            // add the field offset to the byte address to resolve the field address
            auto* fieldAddress = byteAddress + field.m_fieldOffset;

            // cast to type T and return
            return reinterpret_cast<T*>(fieldAddress);

        }
        static const void* getRawFieldPtr(const void* component, const ReflectableField& field) {
            return static_cast<const char*>(component) + field.m_fieldOffset;
        }

    };

    template <typename T>
    concept reflectable_hasReflectedParent = requires {
        Reflection::getComponentFields<T>();
    };

}


