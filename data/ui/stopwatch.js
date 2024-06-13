class Stopwatch {
    constructor() {
      this.startTime = null;
      this.endTime = null;
      this.isRunning = false;
      this.elapsedTime = 0;
      this.isPaused = false;
    }
  
    start() {
      if (!this.isRunning) {
        this.startTime = Date.now();
        this.isRunning = true;
        this.isPaused = false;
      }
    }
  
    stop() {
      if (this.isRunning) {
        this.endTime = Date.now();
        this.isRunning = false;
        this.isPaused = false;
        this.elapsedTime = this.endTime - this.startTime;
      }
    }
  
    pause() {
      if (this.isRunning && !this.isPaused) {
        this.isPaused = true;
        this.elapsedTime += Date.now() - this.startTime;
      }
    }
  
    getElapsedTime() {
      if (this.isRunning && !this.isPaused) {
        const currentTime = Date.now();
        return currentTime - this.startTime;
      } else {
        return this.elapsedTime;
      }
    }
  
    resume() {
      if (this.isPaused) {
        this.startTime = Date.now() - this.elapsedTime;
        this.isPaused = false;
      }
    }
  
    reset() {
      this.startTime = null;
      this.endTime = null;
      this.isRunning = false;
      this.elapsedTime = 0;
      this.isPaused = false;
    }
  }
  