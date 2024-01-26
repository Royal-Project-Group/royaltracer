//
// Created by nikla on 22.10.2023.
//

#ifndef ROYAL_TRACER_EDITORFACADE_H
#define ROYAL_TRACER_EDITORFACADE_H

#include <string>

#include "../interfaces/Colleague.h"
#include "../architecture/Image.h"

/// Facade class that encapsulates the editor subsystem.
/// All calls from outside to inside the editor should pass this class, and vice versa.
/// @inherit Colleague, as a communication partner to the @a RoyalMediator
class EditorFacade : public Colleague {
public:
    virtual ~EditorFacade() = default;

    explicit EditorFacade(RoyalMediator *rm);

    // methods that go inside Editor -> outside Editor

    /// Called when a 3D Object has been added to or updated in the editor.
    /// Is responsible for appropriately storing the object's content in the OBJ Storage System
    /// @see Glossar
    /// @todo Retrieve ID from OBJ Storage System instead of specifying it
    /// @returns if the update was successful
    virtual bool update3DObject(std::string id, const std::string& content) = 0;

    /// Called when a 3D Object has been removed/deleted in the editor.
    /// Is responsible for appropriately deleting that object from the OBJ Storage System.
    /// @see Glossar
    /// @returns if the removal was successful
    virtual bool delete3DObject(std::string id) = 0;

    /// Called when a Material has been added to or updated in the editor.
    /// Is responsible for appropriately storing the object's content in the MTL Storage System
    /// @see Glossar
    /// @returns if the update was successful
    virtual bool updateMaterial(std::string id, const std::string& content)=0;

    /// Called when a Material has been removed/deleted in the editor.
    /// Is responsible for appropriately deleting that object from the MTL Storage System.
    /// @see Glossar
    /// @returns if the removal was successful
    virtual bool deleteMaterial(std::string id)=0;

    /// Called when a Texture has been added to or updated in the editor.
    /// Is responsible for appropriately storing the object's content in the TXR Storage System
    /// @see Glossar
    /// @returns if the update was successful
    virtual bool updateTexture(std::string id, const std::string& content)=0;

    /// Called when a Texture has been removed/deleted in the editor.
    /// Is responsible for appropriately deleting that object from the TXR Storage System.
    /// @see Glossar
    /// @returns if the removal was successful
    virtual bool deleteTexture(std::string id)=0;

    /// Called when the raytracer has been invoked in the editor.
    /// Is responsible for starting the raytracing process.
    /// @returns the raytraced image
    virtual Image initiateRaytracing(std::string config) = 0;

    // methods that go outside Editor -> inside Editor

    // empty so far
};


#endif //ROYAL_TRACER_EDITORFACADE_H
