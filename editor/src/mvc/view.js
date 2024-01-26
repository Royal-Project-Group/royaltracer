import * as THREE from 'three';
import WebGL from 'three/addons/capabilities/WebGL.js';
import Observer from './observer.js';

import {EffectComposer} from 'three/addons/postprocessing/EffectComposer.js';
import {RenderPass} from 'three/addons/postprocessing/RenderPass.js';
import {UnrealBloomPass} from 'three/addons/postprocessing/UnrealBloomPass.js';

import {EnumTypes, TabEditorTypes} from "../enumTypes.js";
import {InspectorComposite} from "../page_elements/inspector/inspectorComposite.js"
import {Top} from "../page_elements/top.js";
import {Popup} from "../page_elements/popup.js";
import {Overview} from "../page_elements/overview.js";
import {TabsController} from "../editor_tabs/tabsController.js";
import {WorkspaceTab} from "../editor_tabs/workspaceTab.js";
import {Animator} from "../editor_tabs/animator.js";


class View extends Observer {

    #renderer;
    #composer;
    #scene;
    #camera;

    #currentElement;

    #objectMap;
    #lightMap;
    #lightHelperMap;
    #materialMap;
    #textureMap;
    #objToMtlMap;

    #controller;
    #inspector
    #top;
    #popup;
    #overview;

    #animator;

    constructor(controller) {
        super();
        this.#objectMap = new Map();
        this.#lightMap = new Map();
        this.#materialMap = new Map();
        this.#textureMap = new Map();
        this.#lightHelperMap = new Map();
        this.#objToMtlMap = new Map();

        this.setupWebGL();
        this.setupPostProcessing();
        this.initalizeAutoDropdownCollapse();

        this.#controller = controller;
        this.#inspector = new InspectorComposite(this, controller);
        this.#top = new Top(this, controller);
        this.#popup = new Popup();


        const overviewTab = new Overview("overviewDiv", this);
        const workspaceTab = new WorkspaceTab("workspace", controller);
        this.#animator = new Animator("animator");

        const tabsController = new TabsController();

        tabsController.addTab(overviewTab, TabEditorTypes.OVERVIEW);
        tabsController.addTab(workspaceTab, TabEditorTypes.WORKSPACE);
        tabsController.addTab(this.#animator, TabEditorTypes.ANIMATOR);

        tabsController.activateTabType(TabEditorTypes.OVERVIEW);

        this.#overview = overviewTab;

        this.#controller.setWorkspace(workspaceTab);
        this.#controller.setAnimator(this.#animator);

        this.animate();
    }

    // Updates the view elements according to the changes in the model
    update(model) {
        this.updateCamera(model);
        this.updateObjects(model);
        this.updateLights(model);
        this.updateMaterials(model);
        this.updateTextures(model);

        //Check if there is an rendered image and display it if so.
        if (model.hasImage()) {
            this.#top.showShowResultButton();
        } else {
            this.#top.hideShowResultButton();
        }

        this.#animator.update(model);
    }

    // Updates the camera according to the updates in the model
    updateCamera(model){
        if(model.getCamera() == null){
            model.setCamera(this.#camera);
        }
        if (model.getCamera().position !== this.#camera.position) {
            this.#camera.position.set(model.getCamera().position.x, model.getCamera().position.y, model.getCamera().position.z);
        }
        if (model.getCamera().rotation === this.#camera.rotation) {
            this.#camera.rotation.set(model.getCamera().rotation.x, model.getCamera().rotation.y, model.getCamera().rotation.z);
        }
    }

    // Update the objects in the scene according to the updates in the model
    updateObjects(model){
        // Iterate over the objects in the model
        for (const [name, object] of model.getObjects().entries()) {
            // Check if 'name' exists in 'objectMap'
            if (!this.#objectMap.has(name)) {
                this.#objectMap.set(name, object);
                this.#scene.add(object);

                this.#overview.appendButton(name);
            }
        }

        // Now, handle the case where objects in 'objectMap' don't exist in model
        for (const [name, existingObject] of this.#objectMap.entries()) {
            if (!model.getObjects().has(name)) {
                // Object in 'objectMap' doesn't exist in model, so remove it
                this.#scene.remove(existingObject);

                // Remove it from 'objectMap' as well
                this.#objectMap.delete(name);
                this.#currentElement = '';
                this.#overview.removeButton(name);

                // Hide the inspector
                this.#inspector.hide();
            }
        }
    }

    // Update the lghts in the scene according to the updates in the model
    updateLights(model){
        // Iterate over the lights in the model
        for (const [name, light] of model.getLights().entries()) {
            // Check if 'name' exists in 'lightMap'
            if (!this.#lightMap.has(name)) {
                this.#lightMap.set(name, light);
                this.#scene.add(light);

                // Wrapping the light in a helper, so it is better visualized in the preview
                // To later have easy access to the helper for a light, the helper is stored in a map with the light name as key.
                let helper;
                if (light instanceof THREE.DirectionalLight) {
                    helper = new THREE.DirectionalLightHelper(light, 1)
                } else {
                    helper = new THREE.PointLightHelper(light, 0.5)
                }
                this.#lightHelperMap.set(name, helper);
                this.#scene.add(helper);

                this.#overview.appendButton(name);
            }
        }

        // Now, handle the case where objects in 'lightMap' don't exist in model
        for (const [name, existingLight] of this.#lightMap.entries()) {
            if (!model.getLights().has(name)) {
                // Object in 'lightMap' doesn't exist in model, so remove it
                this.#scene.remove(existingLight);

                // Remove it from 'lightMap' as well
                this.#lightMap.delete(name);
                this.#overview.removeButton(name);

                // As the light is wrapped in a helper class, we have to also remove the helper.
                this.#scene.remove(this.#lightHelperMap.get(name));
                this.#lightHelperMap.delete(name);

                // Hide the inspector
                this.#inspector.hide();
            }
        }
    }

    // Update the materials in the scene according to the updates in the model
    updateMaterials(model){
        // Iterate over the materials in the model
        for (const [name, material] of model.getMaterials().entries()) {

            // Check if 'name' exists in 'materialMap'
            if (!this.#materialMap.has(name)) {
                this.#materialMap.set(name, material);
                this.#inspector.appendMaterialButton(name);
            }
        }

        // Now, handle the case where materials in 'materialMap' don't exist in model
        for (const [name, existingMaterial] of this.#materialMap.entries()) {
            if (!model.getMaterials().has(name)) {
                // Material in 'materialMap' doesn't exist in model, so remove it
                this.#materialMap.delete(name);
                // remove the button from the dropdown
                this.#inspector.removeMaterialButton(name);
            }
        }
    }

    // Update the textures in the scene according to the updates in the model
    updateTextures(model){
        // Iterate over the textures in the model
        for (const [name, texture] of model.getTextures().entries()) {
            // Check if 'name' exists in 'textureMap'
            if (!this.#textureMap.has(name)) {
                this.#textureMap.set(name, texture);
                this.#inspector.appendTextureButton(name);
            }
        }
    }

    // Initialize the canvas for webgl
    setupWebGL(){
        // Get the container where ThreeJS is displayed
        const viewCanvas = document.getElementById("threeJsCanvas");

        // Check if WebGL is even available
        if (!WebGL.isWebGLAvailable()) {
            const warning = WebGL.getWebGLErrorMessage();
            document.getElementById('container').appendChild(warning);
        }

        // Create the renderer
        this.#renderer = new THREE.WebGLRenderer({
            canvas: viewCanvas,
            antialias: true
        });

        // Initialize the scene with light and a camera
        this.#scene = new THREE.Scene();
        this.#camera = new THREE.PerspectiveCamera(75, viewCanvas.width / viewCanvas.height, 0.1, 1000);
        this.#camera.position.set(0, 0, 5);

        const ambientLight = new THREE.AmbientLight(0xffffff, 0.06); // White light with intensity 0.5
        this.#scene.add(ambientLight);
    }

    // Sets up post processing which is used for the bloom effect for emissive objectsS
    setupPostProcessing() {
        // Create a bloom pass
        const renderScene = new RenderPass(this.#scene, this.#camera);
        const bloomPass = new UnrealBloomPass(new THREE.Vector2(window.innerWidth, window.innerHeight), 1.5, 0.4, 0.85);
        bloomPass.threshold = 0;
        bloomPass.strength = 1;
        bloomPass.radius = 0.1;

        // Create an effect composer and add the passes
        this.#composer = new EffectComposer(this.#renderer);
        this.#composer.addPass(renderScene);
        this.#composer.addPass(bloomPass);
    }

    // Initializes the browser window in a way that all openend dropdowns collapse when there is a click outside of the open dropdown
    initalizeAutoDropdownCollapse() {
        // Closes a dropdown menu if the user clicks outside of it
        window.onclick = function (event) {
            if (!event.target.matches('.drop-btn')) {
                var dropdowns = document.getElementsByClassName("dropdown-content");
                var i;
                for (i = 0; i < dropdowns.length; i++) {
                    var openDropdown = dropdowns[i];
                    if (openDropdown.classList.contains('show')) {
                        openDropdown.classList.remove('show');
                    }
                }
            }
        }
    }

    // Sets the given mtl to the current selected object
    setCurrentMtl(label){
        const mtl = this.#materialMap.get(label);
        // check if all textures are available
        if(!this.texturesAvailable(mtl)) return;
        this.#inspector.updateDropdowns(label);
        // Set the material of the current object to the selected material
        console.log("setting material of " + this.#currentElement + " to " + label)
        this.#controller.setMaterialOfObj(label, this.#currentElement);
        //Add both the mtl and the obj to a map so that we can later get the mtl for a given obj
        this.#objToMtlMap.set(this.#currentElement, label)
    }

    // Checks if the textures referenced in a material are already loaded into the editor
    texturesAvailable(materialCreator) {
        let missingTextures = [];
        // Iterate through each material in materialsInfo
        for (const materialName in materialCreator.materialsInfo) {
            const materialInfo = materialCreator.materialsInfo[materialName];
            // Check for different types of textures (e.g., map_kd, bump, normal)
            ['map_kd', 'bump', 'normal'].forEach(textureType => {
                if (materialInfo[textureType]) {
                    const textureFilename = materialInfo[textureType]; // Assuming the filename is directly stored
                    // Check if the texture is not in the textureMap
                    if (!this.#textureMap.has(textureFilename)) {
                        missingTextures.push(textureFilename);
                    }
                }
            });
        }

        // If there are missing textures, display an alert
        if (missingTextures.length !== 0) {
            let missingTexturesList = Array.from(missingTextures).join(", ");
            alert(`The following textures are not available: ${missingTexturesList}. Import them in the editor, otherwise the model will not be displayed correctly.`);
            return false;
        } else {
            return true;
        }
    }

    // Shows the correct inspector for a given object
    showInspector(name) {
        // Set currently opened element
        this.#currentElement = name;
        this.#inspector.show(name);
    }

    // Keeps the ThreeJS canvas updated
    animate() {
        requestAnimationFrame(() => this.animate());
        this.#composer.render(this.#scene, this.#camera);
    }

    // Returns which type the given element has
    getRenderType(name) {
        if (this.#objectMap.has(name)) return EnumTypes.OBJECT;
        if (this.#lightMap.has(name)) {
            const light = this.#lightMap.get(name);
            return light instanceof THREE.PointLight ? EnumTypes.POINT_LIGHT : EnumTypes.DIRECTIONAL_LIGHT
        }
        return EnumTypes.CAMERA;
    }

    // Remoces all light helpers so they aren't displayed in the preview anymore.
    removeAllHelper(){
        // Remove all Helper objects from the scene, as they should only be displayed when the according object/light is selected
        for (const [name, helper] of this.#lightHelperMap.entries()) {
            this.#scene.remove(helper);
        }
    }

    // Opens the result popup with the given image
    showImage(imageData) {
        this.#popup.showImage(imageData);
    }

    // get current object
    getCurrentObject(){
        return this.#currentElement;
    }

    // opens the result popup
    showPopup(){
        this.#popup.show();
    }

    getCamera(){
        return this.#camera;
    }

    getObject(name){
        return this.#objectMap.get(name);
    }

    getMaterialFromObject(objName){
        return this.#objToMtlMap.get(objName)
    }

    getLight(name){
        return this.#lightMap.get(name);
    }

    getLightHelper(name){
        return this.#lightHelperMap.get(name);
    }

    addToScene(el){
        this.#scene.add(el);
    }

    getCurrentElement(){
        return this.#currentElement;
    }

    // Shows an info message in the bottom of the page
    displayMessage(theme, msg) {
        let container = document.createElement("div");
        let list = document.getElementById("messageList");
        // message color depends on the given theme
        container.className = `p-3 text-${theme} bg-${theme} bg-opacity-10 border border-${theme} rounded mb-4`;
        container.innerHTML = msg;
        container.addEventListener("animationend", (event) => {
            if(event.animationName === "fadeOut")
                list.removeChild(container);
        })
        list.appendChild(container);
    }

}

export default View;
