import * as THREE from 'three';
import WebGL from 'three/addons/capabilities/WebGL.js';
import Observer from './observer.js';

class RequestView extends Observer {

    #objStringMap;
    #controller;
    #materialStringMap;
    #textureStringMap;

    #url;
    constructor(controller) {
        super();
        this.#controller = controller;
        this.#objStringMap = new Map();
        this.#materialStringMap = new Map();
        this.#textureStringMap = new Map();

        fetch('../config.json')
            .then(response => response.json())
            .then(data => {
                this.#url = data.url;
            })

    }

    update(model) {
        ////////////////////////////////// Objects //////////////////////////////////
        // Iterate over the objects in the model
        for (const [name, obj] of model.getObjsString().entries()) {
            // Check if 'name' exists in 'objectMap'
            if (!this.#objStringMap.has(name)) {
                this.#objStringMap.set(name, obj);

                this.putObject(name, obj);

            }
        }

        // Now, handle the case where objects in 'objectMap' don't exist in model
        for (const [name, existingObj] of this.#objStringMap.entries()) {
            if (!model.getObjects().has(name)) {
                // Object in 'objectMap' doesn't exist in model, so remove it
                this.deleteObject(name);

                // Remove it from 'objectMap' as well
                this.#objStringMap.delete(name);
            }
        }

        ////////////////////////////////// Materials //////////////////////////////////
        // Iterate over the materials in the model
        for (const [name, material] of model.getMaterialsString().entries()) {
            // Check if 'name' exists in 'materialMap'
            if (!this.#materialStringMap.has(name)) {
                this.#materialStringMap.set(name, material);
                this.putMaterial(name, material);
            }
        }

        // Now, handle the case where materials in 'materialMap' don't exist in model
        for (const [name, existingMaterial] of this.#materialStringMap.entries()) {
            if (!model.getMaterials().has(name)) {
                // Material in 'materialMap' doesn't exist in model, so remove it
                this.deleteMaterial(name);

                // Remove it from 'materialMap' as well
                this.#materialStringMap.delete(name);
            }
        }

        ////////////////////////////////// Textures //////////////////////////////////
        // Iterate over the textures in the model
        for (const [name, texture] of model.getTexturesString().entries()) {
            // Check if 'name' exists in 'textureMap'
            if (!this.#textureStringMap.has(name)) {
                this.#textureStringMap.set(name, texture);
                this.putTexture(name, texture);
            }
        }

        // Now, handle the case where textures in 'textureMap' don't exist in model
        for (const [name, existingTexture] of this.#textureStringMap.entries()) {
            if (!model.getTextures().has(name)) {
                // Texture in 'textureMap' doesn't exist in model, so remove it
                this.deleteTexture(name);

                // Remove it from 'textureMap' as well
                this.#textureStringMap.delete(name);
            }
        }
    }

    putMaterial(name, material) {
        // request to server

        // create body
        //let body = {
        //    material: material
        //}

        // create request
        let request = new Request(this.#url + "/mtl/" + name, {
            method: "PUT",
            headers: {
                "Content-Type": "text/plain"
            },
            body: material//JSON.stringify(body),
        });

        // send request
        fetch(request)
            .then(response => {
                if (response.ok) {
                    console.log("put material");
                } else {
                   console.log("error putting material");
                }
            })
    }

    // delete a material from the server
    deleteMaterial(name) {
        // request to server

        // create request
        let request = new Request(this.#url + "/mtl/" + name, {
            method: "DELETE",
        });

        // send request
        fetch(request).then(
            response => {
                if (response.ok) {

                } else {
                   console.log("error deleting material");
                }
            }
        )
    }

    // put an object to the server
    putObject(name, obj) {

        //console.log(obj);
        // request to server

        // create body
        //let body = {
        //    obj: obj
        //}

        // create request
        let request = new Request(this.#url + "/obj/" + name, {
            method: "PUT",
            headers: {
                "Content-Type": "text/plain"
            },
            body: obj//JSON.stringify(body),
        });

        // send request
        fetch(request)
            .then(response => {
                response.text().then(text => {
                    // display the response from the backend
                    this.#controller.showBackendMessage(response.ok, text);
                })
            })
    }

    deleteObject(name) {
        // request to server

        // create request
        let request = new Request(this.#url + "/obj/" + name, {
            method: "DELETE",
        });

        // send request
        fetch(request).then(
            response => {
                response.text().then(text => {
                    // display the response from the backend
                    this.#controller.showBackendMessage(response.ok, text);
                })
            }
        )
    }

    putTexture(name, texture) {
        // request to server

        // create body
        //let body = {
        //    texture: texture
        //}

        // create request
        let request = new Request(this.#url + "/txr/" + name, {
            method: "PUT",
            headers: {
                "Content-Type": "text/plain"
            },
            body: texture//JSON.stringify(body),
        });

        // send request
        fetch(request)
            .then(response => {
                if (response.ok) {
                    console.log("put texture");
                } else {
                   console.log("error putting texture");
                }
            })
    }

    deleteTexture(name) {
        // request to server

        // create request
        let request = new Request(this.#url + "/txr/" + name, {
            method: "DELETE",
        });

        // send request
        fetch(request).then(
            response => {
                if (response.ok) {

                } else {
                   console.log("error deleting texture");
                }
            }
        )
    }
}

export default RequestView;