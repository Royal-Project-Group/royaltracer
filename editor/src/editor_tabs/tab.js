class Tab{

    #tabDiv;
    #tabDivId;

    constructor(tabDivId) {
        this.#tabDivId = tabDivId;
        this.#tabDiv = document.getElementById(tabDivId);
    }
    activate() {
        // set tabDiv to not hidden
        this.#tabDiv.style.display = 'block';
    }

    deactivate() {
        // set tabDiv to hidden
        this.#tabDiv.style.display = 'none';
    }

    getTabDivId() {
        return this.#tabDivId;
    }
}
export {Tab};