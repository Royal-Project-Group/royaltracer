// Observable (Subject)
class Subject {
    constructor() {
      this.observers = [];
    }
  
    // Add an observer to the list
    addObserver(observer) {
      if (!this.observers.includes(observer)) {
        this.observers.push(observer);
      }
    }
  
    // Remove an observer from the list
    removeObserver(observer) {
      const observerIndex = this.observers.indexOf(observer);
      if (observerIndex > -1) {
        this.observers.splice(observerIndex, 1);
      }
    }
  
    // Notify all observers about some event
    notify(data) {
      this.observers.forEach(observer => observer.update(data));
    }
}
export default Subject;