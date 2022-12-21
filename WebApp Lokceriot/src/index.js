if ("serviceWorker" in navigator){
navigator.serviceWorker.register("service-worker.js").then(registration => {
    console.log ("Service worker registered");
}).catch(error => {
    console.log("Service worker registration failed");
});

}

