/*
 *
 *  Created on: Dec 20, 2013
 *      Author: vlooys
 */

#include "ComponentManager.h"
#include <dlfcn.h>

namespace OPAQ {

  ComponentManager::ComponentManager() {
    instanceMap = instanceMapType();
    factoryMap = factoryMapType();
  }
  
  ComponentManager::~ComponentManager() {
    // delete all instances
    for (instanceMapType::iterator it = instanceMap.begin(); it != instanceMap.end(); it++) {
      delete it->second;
    }
    instanceMap.clear();
  }
  
  ComponentManager * ComponentManager::getInstance() {
    static ComponentManager instance;
    return &instance;
  }
  
  void ComponentManager::loadPlugin(std::string &pluginName, std::string &filename)
    throw (FailedToLoadPluginException, PluginAlreadyLoadedException) {

    // 1. check if plugin was already loaded
    factoryMapType::iterator it = factoryMap.find(pluginName);
    if (it != factoryMap.end())
      throw PluginAlreadyLoadedException("There already exists a plugin with name " + pluginName);

    // 2. load plugin
    void * handle;
    Component * (*factory)(void);
    char * error;
    handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (!handle)
      throw FailedToLoadPluginException ("Failed to load library with name '" + pluginName
					 + "' from file with name '" + filename + "': " + dlerror());
    //BM funky function handle casting going on here :-)
    factory = (Component* (*)() ) dlsym(handle, "factory");
    if ((error = dlerror()) != NULL) {
      throw FailedToLoadPluginException ("Failed to fetch factory symbol from library with name '"
					 + pluginName + "' in file with name '" + filename + "': " + error);
    }
    factoryMap.insert(std::make_pair(pluginName, factory));
  }


  Component * ComponentManager::createGenericComponent (std::string &componentName,
							std::string &pluginName, TiXmlElement * configuration)
    throw (ComponentAlreadyExistsException, PluginNotFoundException, BadConfigurationException) {

    // 1. check if the component wasn't created previously
    bool ok = false;
    try {
      findComponent(componentName);
    } catch (ComponentNotFoundException &e) {
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

  Component * ComponentManager::findComponent (std::string &name)
    throw (ComponentNotFoundException) {

    instanceMapType::iterator it = instanceMap.find(name);
    if (it == instanceMap.end())
      throw ComponentNotFoundException("Component not found: " + name);
    return it->second;
  }

  Component * ComponentManager::createComponent (std::string &pluginName, TiXmlElement * configuration)
    throw (PluginNotFoundException, BadConfigurationException) {
    factoryMapType::iterator it = factoryMap.find(pluginName);
    if (it == factoryMap.end())
      throw PluginNotFoundException("Plugin not found: " + pluginName);
    Component * component = it->second();
    component->configure(configuration);
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
