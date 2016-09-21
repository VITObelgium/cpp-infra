/*
 *
 *  Created on: Dec 20, 2013
 *      Author: vlooys
 */

#include "ComponentManager.h"
#include <dlfcn.h>
#include "Logger.h"

namespace OPAQ {

  ComponentManager::ComponentManager(IEngine& engine)
  : _engine(engine) {
  }
  
  ComponentManager::~ComponentManager() {
    // delete all instances
    for (instanceMapType::iterator it = instanceMap.begin(); it != instanceMap.end(); it++) {
      delete it->second;
    }
    instanceMap.clear();
  }
  
  // Throws FailedToLoadPluginException PluginAlreadyLoadedException
  void ComponentManager::loadPlugin(std::string &pluginName, std::string &filename) {

    // 1. check if plugin was already loaded
    factoryMapType::iterator it = factoryMap.find(pluginName);
    if (it != factoryMap.end())
      throw PluginAlreadyLoadedException("There already exists a plugin with name " + pluginName);

    // 2. load plugin
    void * handle;
    Component* (*factory)(void*);
    handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (!handle)
      throw FailedToLoadPluginException ("Failed to load library with name '" + pluginName
					 + "' from file with name '" + filename + "': " + dlerror());
    //BM funky function handle casting going on here :-)
    factory = (Component* (*)(void*) ) dlsym(handle, "factory");
    if (factory == nullptr) {
      auto error = dlerror();
      throw FailedToLoadPluginException ("Failed to fetch factory symbol from library with name '"
					 + pluginName + "' in file with name '" + filename + "': " + (error ? error : ""));
    }

    // Create the logger in the application and not in the dll memory space
    factoryMap.insert(std::make_pair(pluginName, factory));
  }


  // Throws ComponentAlreadyExistsException PluginNotFoundException BadConfigurationException
  Component * ComponentManager::createGenericComponent(std::string &componentName,
                                                       std::string &pluginName,
                                                       TiXmlElement* configuration) {

    // 1. check if the component wasn't created previously
    bool ok = false;
    try {
      findComponent(componentName);
    } catch (const ComponentNotFoundException&) {
      ok = true;
    }
    if (!ok)
      throw ComponentAlreadyExistsException("Component already exists: " + componentName);

    // 2. find the factory method for the plugin, create an instance and configure it
    Component * component = createComponent(pluginName, configuration);

    // 2.1 store the component name in the component, to be returned by getName(); 
    component->setName( componentName );

    // 3. store the instance in the instance map
    instanceMap.insert(std::make_pair(componentName, component));
    // 4. and return it
    return component;
  }

  // Throws ComponentNotFoundException
  Component * ComponentManager::findComponent (std::string &name) {

    instanceMapType::iterator it = instanceMap.find(name);
    if (it == instanceMap.end())
      throw ComponentNotFoundException("Component not found: " + name);
    return it->second;
  }

  // Throws PluginNotFoundException BadConfigurationException
  Component * ComponentManager::createComponent (std::string &pluginName, TiXmlElement* configuration) {
    factoryMapType::iterator it = factoryMap.find(pluginName);
    if (it == factoryMap.end())
      throw PluginNotFoundException("Plugin not found: " + pluginName);
    auto sink = Log::getSink();
    Component * component = it->second(&sink);
    component->configure(configuration, _engine);
    return component;
  }

  void ComponentManager::destroyComponent(std::string &componentName) {
    instanceMapType::iterator it = instanceMap.find(componentName);
    if (it == instanceMap.end()) {
      // component not found: nothing to do
    } else {
      Component * component = it->second;
      instanceMap.erase(it);
      delete component;
    }
  }
  
} /* namespace OPAQ */
