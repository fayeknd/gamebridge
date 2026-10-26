#include <layer.hpp>
#include <algorithm>
#include <renderer.hpp>

using namespace Engine;

LayerID Layer::create(const std::string& layerName, int32_t ordering) {
    return LayerRegistry::get().createLayer(layerName, ordering);
}

LayerMask Layer::getMask(const std::string& layerName) {
    return LayerRegistry::get().getLayerViaName(layerName)->m_layerID;
}

LayerID LayerRegistry::createLayer(const std::string &layerName, int32_t ordering) {
    
    // check if a layer by that name already exists
    if (auto layer = getLayerViaName(layerName, false); layer != nullptr) {
        log().logWarning("A layer with name '{}' already exists! Returning that layer's ID instead.", layerName);
        return layer->m_layerID;
    }

    size_t count = m_layerStack.size();
    if (count >= renderer_maxLayers) {
        log().logError("Maximum layer count reached!");
        return 0;
    }

    // next id (doubles as a bitmask)
    LayerID id = (1u << count);

    // create the new layer.
    m_layerStack.push_back({id, layerName, ordering});
    m_layerNameToIDMap.emplace(layerName, id);

    // sort the layer stack by ordering.
    std::sort(m_layerStack.begin(), m_layerStack.end(), [](const Layer& a, const Layer& b) {
        return a.m_ordering < b.m_ordering;
    });

    log().logInfo("Created a new layer with name '{}', ordering of {}.", layerName, ordering);
    return id;
}


Layer* LayerRegistry::getLayerViaName(std::string name, bool logErr) {
    
    // find the LayerID in the map via string
    auto it = m_layerNameToIDMap.find(name);
    if (it != m_layerNameToIDMap.end()) {
        // found it, get the layer via the ID we just found
        return getLayerViaID(it->second);
    }
    if (logErr) log().logError("Could not find a layer with name '{}'.", name);
    return nullptr;
    // didn't find it. 
}

Layer* LayerRegistry::getLayerViaID(LayerID id) {
    auto it = std::find_if(m_layerStack.begin(), m_layerStack.end(), [id](const Layer& l) {
        return l.m_layerID == id;
    });
    
    return (it != m_layerStack.end()) ? &(*it) : nullptr;
}