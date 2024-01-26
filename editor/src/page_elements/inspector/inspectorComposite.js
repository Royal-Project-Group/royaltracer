import {isLight, EnumTypes} from "../../enumTypes.js";
import {Inspector} from "./inspector.js";
import {ObjectInspector} from "./objectInspector.js";
import {CameraInspector} from "./cameraInspector.js";
import {LightInspector} from "./lightInspector.js";


class InspectorComposite extends Inspector {

    #cIns;
    #oIns;
    #lIns;

    constructor(view, controller) {
        super(view, controller);
        this.#oIns = new ObjectInspector(view, controller);
        this.#cIns = new CameraInspector(view, controller);
        this.#lIns = new LightInspector(view, controller);

        // Add the EventListener
        document.getElementById('trashBtn').addEventListener('click', (ev) => {
            let label = this._view.getCurrentElement();
            this._controller.delete(label, this._view.getRenderType(label));
        });

        document.getElementById('cloneBtn').addEventListener('click', (ev) => {
            let label = this._view.getCurrentElement();
            this._controller.clone(label, this._view.getRenderType(label));
        });
    }

    // Checks which element is currently selected and delegates the call to the concrete inspector implementation.
    show(name) {
        const type = this._view.getRenderType(name);
        if (type === EnumTypes.OBJECT) {
            this.#oIns.show(name);
        }
        else if (isLight(type)) {
            this.#lIns.show(name);
        }
        else if (type === EnumTypes.CAMERA) {
            this.#cIns.show(name);
        }
    }

    appendMaterialButton(name){
        this.#oIns.appendMaterialButton(name);
    }

    removeMaterialButton(name){
        this.#oIns.removeMaterialButton(name);
    }

    appendTextureButton(name){
        this.#oIns.appendTextureButton(name);
    }

    updateDropdowns(name){
        this.#oIns.setDropdownText(name);
    }

}
export {InspectorComposite};