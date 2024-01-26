import * as THREE from 'three';
import * as YAML from 'yaml'

class YAMLParser {

    #model;
    #controller;

    #popupContainer;
    #popupContent;
    #closePopupBtn;

    #unresolvedObjects;

    #workspace = null;

    constructor(model, controller) {
        this.#model = model;
        this.#controller = controller;
        this.#unresolvedObjects = [];

        this.#popupContainer = document.getElementById('importPopup');
        this.#popupContent = document.getElementById('objectLoadList');

        this.#closePopupBtn = document.getElementById('closeImportPopup');

        this.#closePopupBtn.addEventListener('click', () => {
            this.#popupContainer.style.display = "none";
            this.#unresolvedObjects = [];
        })
    }

    import(yaml) {
        const scene = YAML.parse(yaml);
        // Extra Args
        if(scene["extraArgs"] != undefined && scene["extraArgs"]["bounces"] != undefined) this.#model.setBounces(scene["extraArgs"]["bounces"]);
        if(scene["extraArgs"] != undefined && scene["extraArgs"]["samples"] != undefined) this.#model.setSamples(scene["extraArgs"]["samples"]);
        // Camera
        this.#model.setWidth(scene["camera"]["width"]);
        this.#model.setHeight(scene["camera"]["height"]);
        this.#model.getCamera().fov = scene["camera"]["fieldOfView"];

        const position = new THREE.Vector3(scene["camera"]["position"]["x"], scene["camera"]["position"]["y"], scene["camera"]["position"]["z"]);
        this.#model.setCameraPosition(position.x, position.y, position.z);

        this.#model.getCamera().up = new THREE.Vector3(scene["camera"]["upVec"]["x"], scene["camera"]["upVec"]["y"], scene["camera"]["upVec"]["z"]);

        const forward = new THREE.Vector3(scene["camera"]["lookAt"]["x"], scene["camera"]["lookAt"]["y"], scene["camera"]["lookAt"]["z"]).normalize();
        this.#model.getCamera().lookAt(forward);

        this.importPointLights(scene["pointLights"]);
        if(scene["extraArgs"] != undefined && scene["extraArgs"]["directionalLights"] != undefined) this.importDirectionalLights(scene["extraArgs"]["directionalLights"]);
        this.importObjects(scene["models"])
    }

    importPointLights(lights) {
        for (const i in lights) {
            const pl = new THREE.PointLight();
            pl.position.x = lights[i]["position"]["x"];
            pl.position.y = lights[i]["position"]["y"];
            pl.position.z = lights[i]["position"]["z"];
            pl.color.r = lights[i]["Ke"]["r"];
            pl.color.g = lights[i]["Ke"]["g"];
            pl.color.b = lights[i]["Ke"]["b"];
            pl.intensity = lights[i]["intensity"];
            this.#model.addLight("point_light", pl);
        }
    }

    importDirectionalLights(lights) {
        for (const i in lights) {
            const dl = new THREE.DirectionalLight();
            dl.position.x = -lights[i]["direction"]["x"];
            dl.position.y = -lights[i]["direction"]["y"];
            dl.position.z = -lights[i]["direction"]["z"];
            dl.color.r = lights[i]["Ke"]["r"];
            dl.color.g = lights[i]["Ke"]["g"];
            dl.color.b = lights[i]["Ke"]["b"];
            dl.intensity = lights[i]["intensity"];
            this.#model.addLight("directional_light", dl);
        }
    }

    importObjects(objects) {
        // check if the needed objects are already loaded in the workspace
        for (let i in objects) {
            // As the config contains the whole path to the file we have to extract only the file name
            //if(this.#workspace.contains(objects[i]["filePath"].replace(/^.*[\\/]/, '')){
            if(this.#workspace === null) this.#workspace = this.#controller.getWorkspace();

            if (this.#workspace.contains(objects[i]["filePath"].replace(/^.*[\\/]/, ''))) {
                const objName = this.#workspace.addObjectFromWorkspaceToSceneAndModel(objects[i]["filePath"].replace(/^.*[\\/]/, ''));
                this.importObjectPosition(objName, objects[i]);
            } else {
                this.#unresolvedObjects.push(objects[i]);
            }
        }

        // objects that are not yet loaded in the workspace now have to be manually loaded by the user.
        this.resolveObjects();
    }

    resolveObjects(){
        if (this.#unresolvedObjects.length > 0) {
            this.#popupContainer.style.display = "flex";
        }else{
            this.#popupContainer.style.display = "none";
            return;
        }

        // Get the list for the objects and clear it
        let list = document.getElementById('unresolvedObjectList');
        list.innerHTML = '';

        for (const i in this.#unresolvedObjects) {
            const data = this.#unresolvedObjects[i];
            let listItem = document.createElement('li');
            listItem.className = 'popup-list-item';

            let text = document.createTextNode(data["filePath"].replace(/^.*[\\/]/, ''));
            listItem.appendChild(text);

            let button = document.createElement('input');
            button.type = 'file';
            button.accept = '.obj';
            button.textContent = 'Load';
            button.className = 'popup-button';
            button.id = i;
            button.addEventListener('input', (event) => {
                const file = event.target.files[0];
                if (file) {
                    document.getElementById('loadingOverlay').style.display = "flex";

                    // Pass a callback function to the controller
                    this.#controller.readObj(file, (name) => {
                        document.getElementById('loadingOverlay').style.display = "none";
                        console.log("deactivating");
                        this.importObjectPosition(name, data)
                    });
                }
                this.#unresolvedObjects.splice(event.target.id, 1);
                this.resolveObjects();
            });

            listItem.appendChild(button);
            list.appendChild(listItem);
        }

        this.#popupContent.appendChild(list); // Add the list to the popupContent
    }

    // fetches the object with the given name from the model and sets the values according to the given data from the config file.
    importObjectPosition(name, data){
        const obj = this.#model.getObject(name);
        obj.position.x = data["position"]["x"];
        obj.position.y = data["position"]["y"];
        obj.position.z = data["position"]["z"];
        obj.rotation.x = data["rotation"]["x"];
        obj.rotation.y = data["rotation"]["y"];
        obj.rotation.z = data["rotation"]["z"];
        obj.scale.x = data["scale"]["x"];
        obj.scale.y = data["scale"]["y"];
        obj.scale.z = data["scale"]["z"];
    }

    export() {
        let yaml = {};

        // add title
        yaml["title"] = "exampleTitle";

        // add models
        yaml["models"] = this.addModels();

        // add PointLights
        yaml["pointLights"] = this.addPointLights();

        // add Camera
        yaml["camera"] = this.addCamera();

        // add Extra Args
        yaml["extraArgs"] = this.addExtraArgs();

        let yamlStr = YAML.stringify(yaml);

        return yamlStr.replaceAll("\'\"", "\"" ).replaceAll("\"\'", "\"" );


    }

    addModels() {
        let models = [];
        let i = 0;
        for (let [name, object] of this.#model.getObjects()) {
            models[i] = {
                filePath: `"${name}"`,
                position: {
                    x: object.position.x,
                    y: object.position.y,
                    z: object.position.z
                },
                rotation: {
                    x: object.rotation.x,
                    y: object.rotation.y,
                    z: object.rotation.z
                },
                scale: {
                    x: object.scale.x,
                    y: object.scale.y,
                    z: object.scale.z
                }
            }
            i++;
        }
        return models;
    }

    addPointLights() {
        let lights = [];
        let i = 0;
        for (let [name, light] of this.#model.getLights()) {
            if(name.indexOf("point") === -1) {
                continue;
            }
            lights[i] = {
                position: {
                    x: light.position.x,
                    y: light.position.y,
                    z: light.position.z
                },
                Ke: {
                    r: parseFloat(light.color.r),
                    g: parseFloat(light.color.g),
                    b: parseFloat(light.color.b)
                },
                intensity: parseFloat(light.intensity)
            }
            i++;
        }

        return lights;
    }

    addCamera() {
        let lookAtVector = new THREE.Vector3(0,0,-1);
        const cam = this.#model.getCamera();
        lookAtVector.applyEuler(cam.rotation)

        console.log("Rotation of camera in euler angles: " + cam.rotation.x + " " + cam.rotation.y + " " + cam.rotation.z);
        console.log("Calculated lookAtVector: " + lookAtVector.x + " " + lookAtVector.y + " " + lookAtVector.z);

        // round because backend cant parse
        lookAtVector.x = Math.round(lookAtVector.x * 1000) / 1000;
        lookAtVector.y = Math.round(lookAtVector.y * 1000) / 1000;
        lookAtVector.z = Math.round(lookAtVector.z * 1000) / 1000;

        lookAtVector.add(cam.position);

        let camera = {
            position: {
                x: cam.position.x,
                y: cam.position.y,
                z: cam.position.z
            },
            lookAt: {
                x: lookAtVector.x,
                y: lookAtVector.y,
                z: lookAtVector.z
            },
            upVec: {
                x: 0, y: 1, z: 0
            },
            fieldOfView: cam.fov,
            width: this.#model.getWidth(),
            height: this.#model.getHeight()
        };
        return camera;
    }

    addExtraArgs() {
        let extras = {
            bounces: this.#model.getBounces(),
            samples: this.#model.getSamples()
        };
        // create comma-separated list of map keys and check if it contains directional light
        if("".concat([...this.#model.getLights().keys()]).indexOf("directional") !== -1) {
            extras["directionalLights"] = this.addDirectionalLights();
        }

        return extras;
    }

    addDirectionalLights() {
        let dirLights = [];
        let i=0;
        for(let [name, light] of this.#model.getLights()) {
            if(name.indexOf("directional") === -1) {
                continue;
            }
            dirLights[i] = {
                direction: {
                    x: -light.position.x,
                    y: -light.position.y,
                    z: -light.position.z
                },
                Ke: {
                    r: parseFloat(light.color.r),
                    g: parseFloat(light.color.g),
                    b: parseFloat(light.color.b)
                },
                intensity: parseFloat(light.intensity)
            }
            i++;
        }

        return dirLights;
    }

}

export {YAMLParser};