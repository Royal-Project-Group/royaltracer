import {Tab} from "../editor_tabs/tab.js";

const Labels = {
    1: 'camera',
    2: 'point_light',
    3: 'directional_light'
};

class Overview extends Tab{

    #view;
    #buttonList;

    constructor(overviewDivId, view) {
        super(overviewDivId);
        this.#view = view;
        this.#buttonList = document.getElementById('buttonList');

        // Append the camera
        this.appendButton("camera");
    }

    // Highlights a button. This is used for showing which element in the overview is currently selected.
    highlightButton(selectedButton) {
        // Delete highlighting for all buttons
        document.querySelectorAll('#buttonList button').forEach(btn => {
            btn.classList.remove('highlighted');
        });

        // Highlight the current button
        selectedButton.classList.add('highlighted');
    }

    // Adds a button/element to the overview.
    appendButton(label) {
        // Create a new button element
        const button = document.createElement('button');

        button.classList.add('OverviewBtn');
        button.setAttribute('id', label)

        // Add Icon according to the type of the element
        const icon = document.createElement('i');
        if (label.startsWith(Labels[1])) {
            icon.classList.add('bi', 'bi-camera-reels');
            button.appendChild(icon);
        } else if (label.startsWith(Labels[2])) {
            icon.classList.add('bi', 'bi-lightbulb');
            button.appendChild(icon);
        } else if (label.startsWith(Labels[3])) {
            icon.classList.add('bi', 'bi-lamp');
            button.appendChild(icon);
        } else {
            button.textContent = label;
        }

        this.highlightButton(button);

        button.addEventListener('click', (ev) => {
            this.#view.showInspector(ev.target.id);
            this.highlightButton(ev.target);
        });

        // Append the button to the div
        this.#buttonList.appendChild(button);
        this.#view.showInspector(label);
    }


    removeButton(label) {
        this.#buttonList.removeChild(document.getElementById(label));
    }

}

export {Overview};