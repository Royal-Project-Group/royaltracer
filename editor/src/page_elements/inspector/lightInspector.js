import {EnumTypes} from "../../enumTypes.js";
import {Inspector} from "./inspector.js"

class LightInspector extends Inspector {

    #colorR;
    #colorG;
    #colorB;

    #intensity;

    constructor(view, controller) {
        super(view, controller);

        this.#colorR = document.getElementById('colorR');
        this.#colorG = document.getElementById('colorG');
        this.#colorB = document.getElementById('colorB');

        this.#intensity = document.getElementById('intensity');
    }

    show(name) {
        super.show(name);
        const light = this._view.getLight(name);

        // Populate the inspector with the current values
        if (this._view.getRenderType(name) === EnumTypes.POINT_LIGHT) {
            this._positionX.value = light.position.x;
            this._positionY.value = light.position.y;
            this._positionZ.value = light.position.z;
        } else {
            this._positionX.value = -light.position.x;
            this._positionY.value = -light.position.y;
            this._positionZ.value = -light.position.z;
            this._positionLabel.innerHTML = "Direction:";
        }

        this.#colorR.value = light.color.r;
        this.#colorG.value = light.color.g;
        this.#colorB.value = light.color.b;

        this.#intensity.value = light.intensity;

        // show the helper for this light
        this._view.addToScene(this._view.getLightHelper(name));

        // Show the necessary inspector parts
        this._lightInspector.style.display = 'block';

        // Add the new EventHandler
        this._applyChangesButton.addEventListener('click', (ev) => {
            this.onApplyChanges(name);
        });
    }

    // Parse the values from the inspector fields and apply them to the according light.
    onApplyChanges(name) {
        super.onApplyChanges();
        let light = this._view.getLight(name);
        // Set color and intensity values
        light.color.r = this.#colorR.value;
        light.color.g = this.#colorG.value;
        light.color.b = this.#colorB.value;
        light.intensity = this.#intensity.value;

        // Set new position values
        if(this._view.getRenderType(name)  === EnumTypes.DIRECTIONAL_LIGHT ){
            light.position.x = -parseFloat(this._positionX.value);
            light.position.y = -parseFloat(this._positionY.value);
            light.position.z = -parseFloat(this._positionZ.value);
        } else {
            light.position.x = parseFloat(this._positionX.value);
            light.position.y = parseFloat(this._positionY.value);
            light.position.z = parseFloat(this._positionZ.value);
        }

        //also update the wrapper for the light.
        this._view.getLightHelper(name).update()
    }

}

export {LightInspector};