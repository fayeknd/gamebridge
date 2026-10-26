#pragma once
#include <gl_context.hpp>
#include <unordered_map>
#include <logger.hpp>

namespace Engine {

    // we will only have 256 layers max
    using LayerID = uint64_t;
    using LayerMask = LayerID;

    constexpr std::string layer_defaultLayer = "DefaultLayer";

    namespace LayerMasks {
        constexpr LayerMask None = 0;           // no layers
        constexpr LayerMask All = 0xFFFFFFFF;   // all layers
    }

    struct Layer {
        LayerID m_layerID;
        std::string m_name;
        int32_t m_ordering;
        bool m_enabled = true;
        bool m_ignoreRaycast = false;
    
        static LayerID create(const std::string& layerName, int32_t ordering);
        static LayerMask getMask(const std::string& layerName);
    };

    // the layer registry holds all the active layers.
    // not using an ID pool for this, because we won't be destroying layers. 
    class LayerRegistry {
    private:

        // the layer stack
        std::vector<Layer> m_layerStack;
        std::unordered_map<std::string, LayerID> m_layerNameToIDMap;
    
        static Logging::Logger& log() {
            static Logging::Logger instance("LAYER");
            return instance;
        }

    public:

        static LayerRegistry& get() {
            static LayerRegistry instance;
            return instance;
        }

        // create a new layer and register it to the layer stack
        LayerID createLayer(const std::string& layerName, int32_t ordering);
        
        // get the layer stack
        const std::vector<Layer>& getLayerStack() const { return m_layerStack; }

        // get a layer via id
        Layer* getLayerViaID(LayerID id);
        Layer* getLayerViaName(std::string name, bool logErr = true);
    }; 
}