import * as THREE from 'three';
import KeyFrame from "./KeyFrame.js";

class AnimatorElement{

    // list of keyFrames
    #keyFrames;

    // both the keyframes we are interpolating between
    #fromKeyFrame;


    // the object we are animating
    #object;
    #objectName;
    constructor(object, objectName) {

        this.#keyFrames = [];
        this.#object = object;
        this.#objectName = objectName;

        this.createAnimatorElement();

        // create first keyframe, with index 0 -> starting position and rotation
        this.addKeyFrame(0, object.position, object.quaternion);

        this.#fromKeyFrame = this.#keyFrames[0];
    }

    addKeyFrame(frameIndex, position, rotation){
        this.#keyFrames.push(new KeyFrame(position, rotation, frameIndex, this.#objectName + "KeyFrameList"));

        // sort the keyframes, otherwise there might be issues when inserting a keyframe before the last one
        this.#keyFrames.sort(function(a, b) {
            return a.getFrameIndex() - b.getFrameIndex();
        });
    }

    interpolate(frameIndex){
        let toKeyFrame = null;
        // we cant interpolate if there is only one keyframe
        if(this.#keyFrames.length === 1){
            return;
        }

        // find the keyframes we are interpolating to
        for(let keyF of this.#keyFrames){
            if(keyF.getFrameIndex() <= this.#fromKeyFrame.getFrameIndex())
                continue;
            else{
               toKeyFrame = keyF;
                break;
            }
        }
        if(toKeyFrame === null){
           return;
        }

        // calculate the percentage of the interpolation
        const percentage = (frameIndex - this.#fromKeyFrame.getFrameIndex()) / (toKeyFrame.getFrameIndex() - this.#fromKeyFrame.getFrameIndex());

        // interpolate the position
        const position = new THREE.Vector3();
        position.lerpVectors(this.#fromKeyFrame.getPosition(), toKeyFrame.getPosition(), percentage);

        // interpolate the rotation
        const rotation = new THREE.Quaternion();
        rotation.slerpQuaternions(this.#fromKeyFrame.getRotation(), toKeyFrame.getRotation(), percentage);



        // set the position and rotation of the object
        this.#object.position.copy(position);
        this.#object.quaternion.copy(rotation);


        // if we are at the end of the interpolation, set the fromKeyFrame to the toKeyFrame
        if(frameIndex === toKeyFrame.getFrameIndex()){
            this.#fromKeyFrame = toKeyFrame;
        }

    }

    createAnimatorElement(){
        // animator element should have a list of keyframes and a button to add a keyframe
        const animatorElement = document.createElement("div");
        animatorElement.classList.add("animatorElement");

        const animatorElementTitle = document.createElement("h3");
        animatorElementTitle.innerHTML = this.#objectName;
        animatorElement.appendChild(animatorElementTitle);

        const keyFrameList = document.createElement("ul");
        keyFrameList.classList.add("keyFrameList");
        keyFrameList.id = this.#objectName + "KeyFrameList";
        animatorElement.appendChild(keyFrameList);

        const addKeyFrameButton = document.createElement("button");
        addKeyFrameButton.innerHTML = "Add KeyFrame";
        addKeyFrameButton.addEventListener("click", () => {
            this.promptKeyFrame();
        })
        animatorElement.appendChild(addKeyFrameButton);

        document.getElementById('animatorElementList').appendChild(animatorElement);

    }

    promptKeyFrame(){
        // pop up a window that allows the user to select the position and rotation of the object
        // maybe alert, the user inpus pos.x, pos.y, pos.z, rot.x, rot.y, rot.z then we parse the input string
        // and create a keyframe with the given values
        const frameIndex = prompt("Please enter the frame index", "0");
        const position = prompt("Please enter the position of the object in the format x,y,z", "0,0,0");
        const rotation = prompt("Please enter the rotation of the object in the format x,y,z", "0,0,0");
        const pos = position.split(",");
        const rot = rotation.split(",");
        const posVec = new THREE.Vector3(parseFloat(pos[0]), parseFloat(pos[1]), parseFloat(pos[2]));
        const rotVec = new THREE.Quaternion();
        // Specify your Euler angles (in radians)
        const euler = new THREE.Euler(
            THREE.MathUtils.degToRad(parseFloat(rot[0])),
            THREE.MathUtils.degToRad(parseFloat(rot[1])),
            THREE.MathUtils.degToRad(parseFloat(rot[2]), 'XYZ'));
        // Set the quaternion using Euler angles
        rotVec.setFromEuler(euler);
        console.log(rotVec);
        //const rotVec = new THREE.Vector3(parseFloat(rot[0]), parseFloat(rot[1]), parseFloat(rot[2]));


        this.addKeyFrame(parseInt(frameIndex), posVec, rotVec);

    }



}
export default AnimatorElement;