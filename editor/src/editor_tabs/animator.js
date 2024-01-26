import {Tab} from "./tab.js";
import * as THREE from 'three';
import AnimatorElement from "./animatorElement/AnimatorElement.js";

class Animator extends Tab{

    numberOfFrames;

    // list of objects that can be animated
    #objectsToSelect;
    #objectsThatAreSelected;
    #objectMap;

    #dropdownMenu;


    #animatorMap;


    constructor(animatorDivId) {
        super(animatorDivId);

        this.#objectsToSelect = [];
        this.#objectsThatAreSelected = [];
        this.#objectMap = new Map();

        this.#animatorMap = new Map();


        document.getElementById('frameInput').addEventListener('input', (e) => {
            this.numberOfFrames = parseInt(e.target.value, 10);
        });


        this.#dropdownMenu = document.getElementById('horizontalAnimatorList');
        this.#dropdownMenu.style.display = 'none';
        document.getElementById('toggleButton').addEventListener('click', function(e) {
            e.target.innerHTML = e.target.innerHTML === 'Hide Scene-Elements <i class="bi bi-caret-down-fill"></i>' ? 'Show Scene-Elements <i class="bi bi-caret-up-fill"></i>' : 'Hide Scene-Elements <i class="bi bi-caret-down-fill"></i>';
            const dropdownMenu = document.getElementById('horizontalAnimatorList');
            dropdownMenu.style.display = dropdownMenu.style.display === 'none' ? 'block' : 'none';
        });

    }

    update(model) {
        // generate new buttons for each object in the model so we can select each object
        const objNameMap = model.getObjsString();
        for(let objName of objNameMap.keys()){
            if(!this.#objectsToSelect.includes(objName)){
                this.#objectsToSelect.push(objName);
                this.showAnimatorButton(objName);
            }
        }

        for (const [name, object] of model.getObjects().entries()) {
            // Check if 'name' exists in 'objectMap'
            if (!this.#objectMap.has(name)) {
                this.#objectMap.set(name, object);
            }
        }

        // add camera to animator
        if(!this.#objectMap.has("Camera")){
            this.addCameraAnimation(model.getCamera());
        }

    }

    showAnimatorButton(objName){
        const button = document.createElement("button");
        button.innerHTML = objName;
        button.className += 'animatorBtn';
        button.addEventListener("click", () => {
            console.log("Clicked on " + objName);
            this.selectObject(objName);
        });
        this.#dropdownMenu.appendChild(button);
    }

    selectObject(objName){
        if(!this.#objectsThatAreSelected.includes(objName)){
            this.#objectsThatAreSelected.push(objName);
            this.showAnimatorElement(objName);
        }
    }

    showAnimatorElement(objName){

        const obj = this.#objectMap.get(objName);

        // create a new animator element for the object
        const animatorElementObject = new AnimatorElement(obj, objName);

        // add the animator element to the animator element list
        this.#animatorMap.set(objName, animatorElementObject);
    }

    animate(frameIndex){
        // for each animator element, animate the object
        for(let animatorElement of this.#animatorMap.values()){
            animatorElement.interpolate(frameIndex);
        }
    }

    getNumberOfFrames(){
        return this.numberOfFrames;
    }

    addCameraAnimation(camera){
        this.showAnimatorButton("Camera");
        this.#objectMap.set("Camera", camera);
    }

}
export {Animator};