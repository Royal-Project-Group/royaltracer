import Subject from "./subject.js";
import * as THREE from 'three';
import {YAMLParser} from "../YAMLParser.js";
import MapParser from "../mvc/mapparser.js";

class Model extends Subject {
    #objectMap;
    #objStringMap;
    #lightMap;
    #materialMap;   // Map of materials (MaterialCreator)
    #materialStringMap; // Map of .mtl files
    #textureMap;
    #textureStringMap;
    #mapparser;

    #cam;

    #width;
    #height;
    #bounces;
    #samples;

    #image; // the last raytraced image result
    #yaml


    constructor(){
        super();
        this.#objectMap = new Map();
        this.#objStringMap = new Map();
        this.#lightMap = new Map();
        this.#materialStringMap = new Map();
        this.#materialMap = new Map();
        this.#textureMap = new Map();
        this.#textureStringMap = new Map();
        this.#mapparser = new MapParser();
        this.#width = 800;
        this.#height = 600;
        this.#bounces = 1;
        this.#samples = 1;
    }


    addObject(label, object, objString){
        const name = this.generateNewName(label, this.getObjects());
        this.#objectMap.set(name, object);
        this.#objStringMap.set(name, objString);

        for(let mtlIndex in object.materialLibraries){
            if(this.#materialMap.has(object.materialLibraries[mtlIndex])){
                this.setMaterial(object.materialLibraries[mtlIndex], name);
            }
        }

        this.notify(this);
        return name;
    }

    setCamera(camera){
        this.#cam = camera;
    }

    setCameraPosition(x, y, z){
        console.log(x + " " + y + " " + z);
        this.#cam.position.x = x;
        this.#cam.position.y = y;
        this.#cam.position.z = z;
        this.notify(this);
    }

    setCameraRotation(x, y, z){
        this.#cam.rotation.x = x;
        this.#cam.rotation.y = y;
        this.#cam.rotation.z = z;
        this.notify(this);
    }

    getCamera(){
        return this.#cam;
    }

    getObject(name) {
        return this.#objectMap.get(name);
    }

    getObjsString(){
        return this.#objStringMap;
    }

    removeObject(name){
        this.#objectMap.delete(name);
        this.#objStringMap.delete(name);
        this.notify(this);
    }

    getObjects(){
        return this.#objectMap;
    }

    // Materials
    // material = THREE.MaterialCreator-object
    // materialString = parsed .mtl-file content
    // Material gets saved to materialString map and as THREE.MaterialCreator-object
    addMaterial(label,material,materialString){
        const name = this.generateNewName(label, this.getMaterials())
        this.#materialMap.set(name, material);
        this.#materialStringMap.set(name, materialString);
        console.log("added material with name to the model: " + name);
        this.notify(this);
    }

    getMaterials(){
        return this.#materialMap;
    }

    // Lights


    addLight(label, light){
        const name = this.generateNewName(label, this.getLights());
        this.#lightMap.set(name, light);
        this.notify(this);
    }

    getLight(name) {
        return this.#lightMap.get(name);
    }

    removeLight(name){
        this.#lightMap.delete(name);
        this.notify(this);
    }

    getLights(){
        return this.#lightMap;
    }

    // Materials
    // function for getting a material from the model
    getMaterial(name){
        return this.#materialMap.get(name);
    }

    // function for setting a material to an object
    setMaterial(materialName, objectName){

        console.log("setting material " + materialName + " to object " + objectName);
        const mtl = this.#materialMap.get(materialName);
        const obj = this.#objectMap.get(objectName);

        if(obj == null){
            console.log("Materials can only be assigned to objects!");
            return;
        }
        console.log(obj);

        // Traverse throw all childs of the MaterialCreator-object
        obj.traverse((child) => {
            if (child instanceof THREE.Mesh) {
                // check if the child has one material or an array of materials
                // sometimes the child has an array of materials, sometimes it has only one material
                const materials = Array.isArray(child.material) ? child.material : [child.material];
                console.log(materials);
                // iterate over all materials of the child
                materials.forEach((material, i) => {
                    const materialName = material.name;
                    if (mtl.materials[materialName]) {
                        materials[i] = mtl.materials[materialName];
                        // if the material has a texture, set the texture to the material
                        if (materials[i].map !== undefined) {
                            this.setTexture(mtl, materialName);
                        }
                        // if the material has a bump map, set the bump map to the material
                        if (materials[i].bumpMap !== undefined) {
                            this.setBumpMap(mtl, materialName);
                        }
                        // if the material has a normal map, set the normal map to the material
                        if (materials[i].normalMap !== undefined) {
                            this.setNormalMap(mtl, materialName);
                        }
                    } else {
                        // if the material is not found, log an error
                        console.log(`Material ${materialName} in ${mtl.name} not found`);
                        console.log(material.id);
                    }
                    // set the name of the child
                    child.material = Array.isArray(child.material) ? materials : materials[0];
                });
            }
        });
        this.notify(this);
    }

    // getting the materials string map
    getMaterialsString(){
        return this.#materialStringMap;
    }


    // function for resetting the material of an object
    resetMaterialOfObject(objectName) {
        console.log(objectName);
        let obj = this.#objectMap.get(objectName);
        if (!obj) {
            console.error(`Object ${objectName} not found in the model.`);
            return;
        }
        // Traverse throw all childs of the MaterialCreator-object
        obj.traverse((child) => {
            if (child instanceof THREE.Mesh) {
                const materials = Array.isArray(child.material) ? child.material : [child.material];
                console.log(materials);
                materials.forEach((material, i) => {
                    // Save the name of the material of the child before resetting the material that the material name
                    // can be restored after resetting the material
                    const childMaterialName = materials[i].name;
                    materials[i] = new THREE.MeshPhongMaterial();
                    materials[i].name = childMaterialName;
                    // Reset the texture, bump map, and normal map of the material
                    ['map', 'bumpMap', 'normalMap'].forEach(mapType => {
                        if (material[mapType]) {
                            material[mapType].dispose();
                            material[mapType] = null;
                        }
                    });
                });
                child.material = Array.isArray(child.material) ? materials : materials[0];
            }
        });
        console.log("reset material to object " + objectName);
        this.notify(this);
    }

    // Setting the diffuse texture of a material
    setTexture(mtl, materialName){
        const mapInfo = mtl.materialsInfo[materialName].map_kd;
        if (!mapInfo) {
            console.log(`Map info for map_kd of material ${materialName} is undefined or null.`);
            return;
        }
        // Parse the map_kd data, which contains the filename, scale, offset, and range
        let mapKdData = this.#mapparser.parseMap(mtl.materialsInfo[materialName].map_kd);
        let filename = mapKdData.get("filename");
        if (filename !== null) {
            if (mtl.materials[materialName].map != null && this.#textureMap.has(filename)) {
                let texture = this.#textureMap.get(filename);

                // Apply scale, offset, and range if available
                /* This is currently not implemented in the kernel, so atm we will just ignore it
                let scale = mapKdData.get("scale");
                let offset = mapKdData.get("offset");
                let range = mapKdData.get("range");
                // apply scale, offset, and range if available
                if (scale && scale.length >= 2) {
                    texture.repeat.set(parseFloat(scale[0]), parseFloat(scale[1]));
                }
                if (offset && offset.length >= 2) {
                    texture.offset.set(parseFloat(offset[0]), parseFloat(offset[1]));
                }
                if (range && range.length > 0) {
                    texture.anisotropy = parseFloat(range[0]);
                }
                 */

                mtl.materials[materialName].map = texture;
                console.log(`${filename} found in textureMap`);
            } else if (mtl.materials[materialName].map != null && !this.#textureMap.has(filename)) {
                console.log(`${filename} not found in textureMap`);
            }
        } else {
            console.log("no texture found for material ");
        }
    }

    // Setting the bump map of a material
    setBumpMap(mtl, materialName) {
        const mapInfo = mtl.materialsInfo[materialName].bump;
        if (!mapInfo) {
            console.log(`Map info for bumpMap of material ${materialName} is undefined or null.`);
            return;
        }
        let mapBumpData = this.#mapparser.parseMap(mtl.materialsInfo[materialName].bump);
        let filename = mapBumpData.get("filename");
        if (filename !== null && mtl.materials[materialName].bumpMap != null) {
            if (this.#textureMap.has(filename)) {
                let texture = this.#textureMap.get(filename);
                // Apply scale, offset, and bump if available
                /* This is currently not implemented in the kernel, so atm we will just ignore it
                let scale = mapBumpData.get("scale");
                let offset = mapBumpData.get("offset");
                let bump = mapBumpData.get("bump");
                if (scale && scale.length >= 2) {
                    texture.repeat.set(parseFloat(scale[0]), parseFloat(scale[1]));
                }
                if (offset && offset.length >= 2) {
                    texture.offset.set(parseFloat(offset[0]), parseFloat(offset[1]));
                }
                if (bump && bump.length > 0) {
                    // Adjust bumpMap properties if needed
                    texture.bumpScale = parseFloat(bump[0]);
                }

                 */
                mtl.materials[materialName].bumpMap = texture;
                console.log(`${filename} found in textureMap for bump map`);
            } else {
                console.log(`${filename} not found in textureMap for bump map`);
            }
        }
    }

    // Setting the normal map of a material
    setNormalMap(mtl, materialName) {
        const mapInfo = mtl.materialsInfo[materialName].normalMap;
        if (!mapInfo) {
            console.log(`Map info for normalMap of material ${materialName} is undefined or null.`);
            return;
        }
        let mapNormalData = this.#mapparser.parseMap(mtl.materialsInfo[materialName].normalMap);
        let filename = mapNormalData.get("filename");
        if (filename !== null && mtl.materials[materialName].normalMap != null) {
            if (this.#textureMap.has(filename)) {
                let texture = this.#textureMap.get(filename);
                // Apply scale, offset, and normal map specific properties if available
                /* This is currently not implemented in the kernel, so atm we will just ignore it
                let scale = mapNormalData.get("scale");
                let offset = mapNormalData.get("offset");
                if (scale && scale.length >= 2) {
                    texture.repeat.set(parseFloat(scale[0]), parseFloat(scale[1]));
                }
                if (offset && offset.length >= 2) {
                    texture.offset.set(parseFloat(offset[0]), parseFloat(offset[1]));
                }
                */
                mtl.materials[materialName].normalMap = texture;
                console.log(`${filename} found in textureMap for normal map`);
            } else {
                console.log(`${filename} not found in textureMap for normal map`);
            }
        }
    }

    // delete a material from the model
    deleteMaterial(name){
        this.#materialMap.delete(name);
        this.notify(this);
    }

    // function for adding a texture to the model
    // texture is a THREE.Texture and textureString is the contents of the texture file
    addTexture(label, texture, textureString){
        const name = this.generateNewName(label, this.getTextures())
        this.#textureMap.set(name, texture);
        this.#textureStringMap.set(name, textureString);
        console.log("added texture with name to the model: " + name);
        this.notify(this);
    }

    // function for getting all textures from the model
    getTextures(){
        return this.#textureMap;
    }

    getTexturesString(){
        return this.#textureStringMap;
    }

    // get a specific texture from the model
    getTexture(name){
        return this.#textureMap.get(name);
    }

    getTextureString(name){
        return this.#textureStringMap.get(name);
    }

    setImage(image){
        this.#image = image;
        this.notify(this);
    }

    hasImage(){
        return this.#image != null;
    }

    getWidth() {
        return this.#width;
    }

    setWidth(width) {
        this.#width = width;
    }

    getHeight() {
        return this.#height;
    }

    setHeight(height) {
        this.#height = height;
    }

    getBounces() {
        return this.#bounces;
    }

    setBounces(bounces) {
        this.#bounces = bounces;
    }

    getSamples(){
        return this.#samples;
    }

    setSamples(samples) {
        this.#samples = samples;
    }

    asYaml(){
        return this.#yaml.export();
    }

    fromYaml(yamlContent){
        this.#yaml.import(yamlContent);
    }

    // Generates a new name based on a base that is not already in the model.
    generateNewName(base, list){
        let nameAsIndex = base;
        let counter = 0;

        if(list === undefined) return base;

        while(list.has(nameAsIndex)){
            counter += 1;
            nameAsIndex = base + '(' + counter + ')';
        }

        return nameAsIndex
    }

    setYAMLParser(parser){
        this.#yaml = parser;
    }

}
export default Model;