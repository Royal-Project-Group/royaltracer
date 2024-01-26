// class that is responsible for activating tabs
import {TabEditorTypes} from "../enumTypes.js";
import {Animator} from "./animator.js";
import {WorkspaceTab} from "./workspaceTab.js";
import {Overview} from "../page_elements/overview.js";

class TabsController {

    #tabTypesMap;
   #tabsMap;
   #activeTab;


    constructor() {
        this.#tabsMap = new Map();
        this.#tabTypesMap = new Map();
        this.#activeTab = null;
    }

    addTab(tab, tabType) {

        switch (tabType) {
            case TabEditorTypes.OVERVIEW:
                document.getElementById("overviewTabBtn").addEventListener("click", () => {
                    this.activateTabType(TabEditorTypes.OVERVIEW)
                });
                break;
            case TabEditorTypes.WORKSPACE:
                document.getElementById("workspaceTabBtn").addEventListener("click", () => {
                    this.activateTabType(TabEditorTypes.WORKSPACE);
                });
                break;
            case TabEditorTypes.ANIMATOR:
                document.getElementById("animatorBtn").addEventListener("click", () => {
                    this.activateTabType(TabEditorTypes.ANIMATOR);
                    alert("Warning: This feature is still under development and therefore experimental.")
                });
                break;
        }

        this.#tabsMap.set(tab.getTabDivId(), tab);
        this.#tabTypesMap.set(tabType, tab);
    }

    activateTabType(tabType) {
        try {
            this.deactivateActiveTab();
        }
        catch (e) {
            console.log("No active tab to deactivate");
        }

        this.#activeTab = this.#tabTypesMap.get(tabType);
        this.#activeTab.activate();
    }

    deactivateActiveTab() {
        this.#activeTab.deactivate();
    }

}
export {TabsController};