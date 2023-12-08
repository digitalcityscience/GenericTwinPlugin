# GenericTwin - A framework for building urban digital twins for Unreal Engine 5

## Motivation
Urban digital twin becoming more and more popular and can be used for a variety of use cases. Each use case might have some unique features and properties but the base requirements of the underlying digital twin are often the same: visualising geospatial or geolocated data. Instead of creating a separate digital twin from scracth for each use case, a generic digital twin framework has been implemented to setup a digital twin in minutes which then can be customized further for any particular use cases. The generic twin framework offers core and additional features in plugins. A plugin can be considered as a building block adding some kind of functionality. The final digital twin can be deployed as a desktop application, a VR application or as a browser-based solution.

## Key concepts

The generic twin framework is built around the following key concepts:
- fully layer based, lightweight core framework which is extendible via Unreal's plugin system
- data source agnostic
- dynamic UI framework
- global internal data storage
- centralized coordinate transformation
- local file cache
- generic simulation API
- fully automated creation of a digital twin application for non-developers

### Layers
The most important concept is that the generic digital twin framework is fully layer-based. Everything that needs to be visualised must be implemented in some layer. Even the UI is represented as layer. Available layers are registered with a layer repository at startup. Layers are added to the view either at startup based on a configuration file or can be added at runtime. Dependencies between layers can be established, so that for instance one layer delays it's loading for a certain area until another layer has finished loading all the data for that area. Cut-Outs can defined on a per layer basis to setup regions where a layer is not supposed to contribute any content. Layers can be queried for static and dynamic parameters which is used to dynamically create an UI which allows to modify parameters at runtime. Static parameters are used to setup a UI form when a layer requiring parameters at creation time is added at runtime.

A layer is fully responsible for loading and storing its required data. That way each layer can choose the best strategy for the data it handles.

A generic tile manager is already implemented which supports:
- streaming of data
- view frustum culling of tiles currently not visible
- allows for GPU and heap memory budget for garbage collection

## Architectural overview
Generic twin framework is developed for Unreal Engine 5. Most of the code is written in C++. All core logic is implemented in Unreal Engine plugins and all additional features are also best implemented in plugins. That way the generic twin framework can be added easily to existing projects as well. The actual Unreal project is responsible for:
- deployment
- project/use case specific implementation, such as input handling and camera control, project-specific UI etc.
- business logic


### GenericTwinPlugin
GenericTwinPlugin contains the core functionality of the framework. Adding the GenericTwinPlugin to a project allows you to implement your own layers. 

It implements the following core features:
- layer management
- tile management
- tile based streaming layer
- generic UI framework
- coordinate transformation
- local file cache
- texture cache
- texture atlas builder
- dynamic mesh compopnent and dynamic mesh actor
- image importer
- model importer

It already provides some common layers:
- StaticModelLayer, allowing to place static 3D models in the world
- StreamingModelLayer, allowing for streaming of 3D meshes based on a tiling scheme i.e. pre-processed terrain tiles as 3D meshes
