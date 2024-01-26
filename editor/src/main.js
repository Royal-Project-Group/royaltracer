import View from './mvc/view.js';
import RequestView from "./mvc/requestView.js";
import Controller from './mvc/controller.js';
import Model from './mvc/model.js';
import {YAMLParser} from "./YAMLParser.js";
import {TabsController} from "./editor_tabs/tabsController.js";

const model = new Model();
const controller = new Controller(model);
model.setYAMLParser(new YAMLParser(model, controller));
const view = new View(controller);
const requestView = new RequestView(controller);


controller.setView(view);
model.addObserver(view);
model.addObserver(requestView);

