
const musicalNoteIcon = '\u{266A}';
const networkedIcon = '\u{2301}';
const computerIcon = '\u{1F4BB}';
const midiIcon = '\u{1F3B9}';

var gateway = `ws://${window.location.hostname}:${window.location.port}/ws`;
var websocket;

var _currentEffectNo = -1;

var brightnessSliderIsActive = false;

var selectedTabIndex = 0;

var controlsTabIndex = 0;
var devicesTabIndex = 6;

/***
 * 
 * 
 * 
 */

class Device
{
    ip = "127.0.0.1";
    name = "unknown";
    type = 0;

    constructor(ip, type, name) {
        this.ip = ip;
        this.type = type;
        this.name = name;
    }
}

class DeviceList
{
    devices = new Map();

    constructor() {}

    addDevice(device) {
        this.devices.set(device.ip, device);
    }

    hasDevice(ip) {
        return this.devices.has(ip);
    }

    getDevice(ip) {
        return this.devices.get(ip);
    }

    removeDevice(ip) {
        this.devices.delete(ip);
    }

    count() {
        return this.devices.size;
    }
}


class AudioLevelsData
{
    constructor(volumes, frequencyData) {
        this._volumes = volumes;
        this._frequencyData = frequencyData;
        this._arrivalTime = Date.now();
    }

    getVolumes() {
        return this._volumes;
    }

    getFrequencyData() {
        return this._frequencyData;
    }

    getArrivalTime() {
        return this._arrivalTime;
    }
}

// Audio Levels Interpolation Motes:
//
// Strategy. Have a timer. ticking at interpolated fresh rate.
// This (only this) actually draws the histogram.
// Look at old and new. Old means already rendered.
//
// (0) if new is null, do nothing.
//
// (i) span = new - old (timestaps).
//
// (ii) If now > (new + span )
//         then time to draw new, and do old=new. (Make new as null);
//
// (iii) else /* interpolate */
//          fraction between = fraction of now() between old and new.
//          interpolate and draw.
//          leave old and new as they are.

var audioSmoothingEnabled = true;
var audioSmoothingInterval = 50;
var previousAudioLevels = new AudioLevelsData([0, 0], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
var latestAudioLevels = new AudioLevelsData([0, 0], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
var audioRenderingTimer = null;

var asyncAudioRenderingCount = 0;

function toggleSmoothRendering()
{
    if (audioSmoothingEnabled) {
        audioSmoothingEnabled = false;
        clearInterval(audioRenderingTimer);
        audioRenderingTimer = null;
    } else {
        audioSmoothingEnabled = true;
        audioRenderingTimer = setInterval(asyncRenderAudioLevels, audioSmoothingInterval);
    }
}

function initAudioRendering()
{
    if (audioSmoothingEnabled) {
        // TODO: Start interval time to do updates.
        audioRenderingTimer = setInterval(asyncRenderAudioLevels, audioSmoothingInterval);
    }
}

function updateAudioLevels(volumes, frequencyData)
{
    if (!audioSmoothingEnabled) {
        drawAudioHistograms(volumes, frequencyData, false);
    }
    
    else {

        // Make sure we've reached the final point for the previous
        // values.
        drawAudioHistograms(latestAudioLevels, latestAudioLevels, false);

        previousAudioLevels = latestAudioLevels;
        latestAudioLevels = new AudioLevelsData(volumes, frequencyData); 
    }
}

function asyncRenderAudioLevels()
{
    if (audioSmoothingEnabled)
    {
        var now = Date.now();
        var previousTime = previousAudioLevels.getArrivalTime();
        var latestTime = latestAudioLevels.getArrivalTime();
    
        if (previousTime == latestTime) {
            // Then we're up to date with rendering.
        }

        else 
        {
            var totalSpan = latestTime - previousTime;

            if (now > (latestTime + totalSpan)) {
                drawAudioHistograms(latestAudioLevels.getVolumes(), latestAudioLevels.getFrequencyData(), false);
                previousAudioLevels = latestAudioLevels;
            }

            else {
                // Perform interpolation.
                var span = now - latestTime;
                var fraction = span / totalSpan;

                var volumes = interpolateValueArrays(
                    previousAudioLevels.getVolumes(),
                    latestAudioLevels.getVolumes(),
                    fraction
                );

                var freqData = interpolateValueArrays(
                    previousAudioLevels.getFrequencyData(),
                    latestAudioLevels.getFrequencyData(),
                    fraction
                );

                drawAudioHistograms(volumes, freqData, false);
            }
        }
    }

    /*
    else {
        drawAudioHistograms(latestAudioLevels.getVolumes(), latestAudioLevels.getFrequencyData());
    }
    */
    
    ++asyncAudioRenderingCount;
}

function interpolateValueArrays(arr1, arr2, fraction) 
{
    var n1 = arr1.lenght;
    var n2 = arr2.length;

    var n = (n1 < n2) ? n1 : n2;
    var res = new Array(n);

    for (let i = 0; i < n; i++) {
        var val = interpolateValue(arr1[i], arr2[i], fraction);
        res[i] = val;
    }

    return res;
}


function interpolateValue(val1, val2, fraction) {
    var span = val2 - val1;
    return val1 + span * fraction;
}

var allDevices = new DeviceList();

function onBrightnessSliderActive(isActive) {
    brightnessSliderIsActive = isActive;
}

function onBrightnessChangedLocally(value) {
    // Sending brightness values as the slider is tracked is 
    // a bit too strenuous for a ESP32 web socket.
    /**
    if (brightnessSliderIsActive) {
        changeBrightness();
    }
    **/
}

function onSelectNotificationPeriod(notificationNo, secs) {
    saveSetting("notification-" + notificationNo + "-period", secs);
}

function refreshPage()
{
    var url = window.location.href;
    window.location.replace(url);
}

// Simple changing of the light colour.

// Have separate callbacks for onInput nd onChange.
// Always report the final value, but only report the changing value now-and-then (every 0.2s?).
// Avoid flooding the ESP32 with WS messages of any colour changes, to either the primary
// lamp light, or an effect colour property.

// TODO: Note: we have contention between local updates to the color selector and those
// coming i over the WS. Ideally, we only want to take note of the WS one when things 
// come to a rest (e.g. not had a local change for some time T).

var trackColourDuringSelection = true;
var lastColourSelectUpdateTime = new Date();
var lastColourUpdateInterval = 200; // ms (so 5Hz).

var root = document.querySelector(":root");

/*
var audioHistogramBackground = getComputedStyle(root).getPropertyValue("--audioHistogramBackground");
var volumeUnlitRGB = getComputedStyle(root).getPropertyValue('--volumeUnlitRGB');
var volumeLitRGB = getComputedStyle(root).getPropertyValue('--volumeLitRGB');
var frequenciesUnlitRGB = getComputedStyle(root).getPropertyValue("--frequenciesUnlitRGB");
var frequenciesLitRGB = getComputedStyle(root).getPropertyValue("--frequenciesLitRGB");
*/

/*
var audioHistogramBackground;
var volumeUnlitRGB;
var volumeLitRGB;
var frequenciesUnlitRGB;
var frequenciesLitRGB;
*/

var audioHistogramBackground = getComputedStyle(root).getPropertyValue("--audioHistogramBackground");
var volumeUnlitRGB = getComputedStyle(root).getPropertyValue('--volumeUnlitRGB');
var volumeLitRGB = getComputedStyle(root).getPropertyValue('--volumeLitRGB');
var frequenciesUnlitRGB = getComputedStyle(root).getPropertyValue("--frequenciesUnlitRGB");
var frequenciesLitRGB = getComputedStyle(root).getPropertyValue("--frequenciesLitRGB");

var secondaryBackground = getComputedStyle(root).getPropertyValue("--secondary-background");
var onSecondaryBackground = getComputedStyle(root).getPropertyValue("--on-secondary-background");

var gaugeValueColor = getComputedStyle(root).getPropertyValue("--gauge-value-color");

function readRootVars()
{
    audioHistogramBackground = getComputedStyle(root).getPropertyValue("--audioHistogramBackground");
    volumeUnlitRGB = getComputedStyle(root).getPropertyValue('--volumeUnlitRGB');
    volumeLitRGB = getComputedStyle(root).getPropertyValue('--volumeLitRGB');
    frequenciesUnlitRGB = getComputedStyle(root).getPropertyValue("--frequenciesUnlitRGB");
    frequenciesLitRGB = getComputedStyle(root).getPropertyValue("--frequenciesLitRGB");

    secondaryBackground = getComputedStyle(root).getPropertyValue("--secondary-background");
    onSecondaryBackground = getComputedStyle(root).getPropertyValue("--on-secondary-background");

    gaugeValueColor = getComputedStyle(root).getPropertyValue("--gauge-value-color");
}

function logRootVars()
{
    console.log("#### audio histogram background is: " + audioHistogramBackground);
    console.log("#### audio histogram vol unlit is: " + volumeUnlitRGB);
    console.log("#### audio histogram vol lit is: " + volumeLitRGB);
    console.log("#### audio histogram freq unlit is: " + frequenciesUnlitRGB);
    console.log("#### audio histogram freq lit is: " + frequenciesLitRGB);
    console.log("### gauge value text colour is: " + gaugeValueColor);
}


function addPresetOptions(options, labelPrefix, valuePrefix, num, base) {
    for (let i = 0; i < num; i++) {
        addPresetOption(options, labelPrefix + (i + 1), valuePrefix + (base + i));
    }
}

function addPresetOption(options, label, value) {
    console.debug("adding option for '" + label + "' with value of " + value);
    options.add(new Option(label, value));
}

function onTabClick(evt, tabLinkIndex, requiredTabId) {
    tabLink = evt.currentTarget;
    selectTab(tabLinkIndex, tabLink, requiredTabId);
}

function selectTab(tabLinkIndex, tabLink, requiredTabId) {
    activateTab(requiredTabId, tabLink);
    setCookie("selectedTabIndex", tabLinkIndex);
    selectedTabIndex = tabLinkIndex;
}

function activateTab(requiredTabId, tabLink)
{
    var i, tabbedContent, tablinks;

    // Get all elements with class="tabcontent" and hide them.
    // We only need one of them shown/visible at a time.

    tabbedContent = document.getElementsByClassName("tab-container");

    for (i = 0; i < tabbedContent.length; i++) {
        tabbedContent[i].style.display = "none";
    }

    // Get all elements with class="tab-link" and remove the class "active".
    // We only need one of them active at a time.

    tablinks = document.getElementsByClassName("tab-link");

    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }

    // Show the current tab and make the associated tab content visible.
    
    document.getElementById(requiredTabId).style.display = "block";
    tabLink.className += " active";
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    console.log('ws gateway is ' + gateway);
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; 
    websocket.onerror = onError;
}

function onOpen(event) {
    console.log('Connection opened');
    setOnlineState(true);
}

function onClose(event) {
    console.log('Connection closed');
    setOnlineState(false);
    setTimeout(initWebSocket, 2000);  // was 1000, sometimes caused issues. Maybe need it to back out 1000, 2000, 4000, 8000, 8k, 8k, until connects.
}

function onError(error) {
    console.log('connection error is ' + error.data);
}

function onMessage(event)
{
    var message = String(event.data);

    // The message may contain more than one command.
    // Commands are separated by a tilda ('~') sequence.

    var commandDelimiter = "~";

    var commands = message.split(commandDelimiter);
    var n = commands.length;

    if (n > 1) {
        console.log("ws: The following %d messages were aggregated.", n);
    }

    for (var i = 0; i < n; i++) {
        var command = commands[i].trim();
        if (command.length > 0) {
            console.log("ws: rx '" +command+ "'");
            onSingleMessage(command);
        }
    }
}

function onSingleMessage(message)
{
    parts = message.split(" ", 1);

    if (parts.length > 0)
    {
        cmd = parts[0];
        cmdLen = cmd.length;

        argsStr = message.substring(cmdLen).trim();

        if (cmd == "power") {
            args = argsStr.split(" ");
            onPowerMessage(args);
        }

        else if (cmd == "fan-speed") {
            args = argsStr.split(" ");
            onFanSpeedMessage(args);
        }

        else if (cmd == "temperature") {
            args = argsStr.split(" ");
            onTemperatureMessage(args);
        }

        else if (cmd == "humidity") {
            args = argsStr.split(" ");
            onHumidityMessage(args);
        }

        else if (cmd == "air-pressure") {
            args = argsStr.split(" ");
            onAirPressureMessage(args);
        }

        else if (cmd == "iaq") {
            args = argsStr.split(" ");
            onAirQualityMessage(args);
        }

        else if (cmd == "iaq-ready") {
            args = argsStr.split(" ");
            onAirQualityReadyMessage(args);
        }

        else if (cmd == "tvoc") {
            args = argsStr.split(" ");
            onTVOCMessage(args);
        }

        else if (cmd == "co2") {
            args = argsStr.split(" ");
            onCO2Message(args);
        }

        else if (cmd == "heap") {
            args = argsStr.split(" ");
            onHeapUsage(args);
        }

        else if (cmd == "device") {
            args = argsStr.split(" ");
            onDeviceDetectionMessage(args);
        }

        else if (cmd == "setting") {
            args = argsStr.split(" ");
            onSettingMessage(args);
        }

        /*

        else if (cmd == "level") {
            args = argsStr.split(" ");
            onBrightnessMessage(args);
        }

        else if (cmd == "rgb") {
            args = argsStr.split(" ");
            onSetLightRGBMessage(args);
        }

        else if (cmd == "load-preset") {
            args = argsStr.split(" ");
            onLoadPresetMessage(args);
        }

        else if (cmd == "effect") {
            args = argsStr.split(" ");
            onSelectEffectMessage(args);
        }

        else if (cmd == "effect-setting") {
            onEffectSettingsMessage(argsStr);
        }

        else if (cmd == "audio-setting") {
            onAudioSettingMessage(argsStr);
        }

        else if (cmd == "audio-levels") {
            args = argsStr.split(" ");
            onAudioLevelsMessage(args);
        }
        
        else if (cmd == "timer") {
            args = argsStr.split(" ");
            onTimerMessage(args);
        }

        else if (cmd == "status") {
            onStatusMessage(argsStr);
        }

        */

        else {
            console.log("ws: unknown command '" + cmd + "'");
        }
    }

    else {
        console.log("Unexpected from websocket: '" + message + "'");
    }
}

function onSettingMessage(args) 
{
    if (args.length > 0)
    {
        setting = args[0];

        if (args.length > 1)
        {
            let value = args[1];

            if (setting == "transition-type") {
                onSettingsTransitionType(value);
            }

            else if (setting == "transition-time") {
                onSettingsTransitionTime(value);
            }

            else if (setting == "on-off-time") {
                onSettingsPowerOnOffTime(value);
            }

            else if (setting == "fade-in-time") {
                onSettingsFadeInTime(value);
            }

            else if (setting == "fade-out-time") {
                onSettingsFadeOutTime(value);
            }

            else if (setting == "gamma-correct") {
                onSettingGammaCorrect(value);
            }

            else if (setting == "preview") {
                onSettingShowPreview(value);
            }

            else if (setting == "boot-power") {    // TODO CHECK message.
                onSettingBootPower(value);
            }

            else if (setting == "boot-brightness") {    // TODO CHECK message.
                onSettingBootBrightness(value);
            }

            else if (setting == "notifications") {
                onSettingNotificationsEnabled(value);
            }

            else if (setting == "notification-1-period") {
                onSettingNotificationsPeriod(1, value);
            }

            else if (setting == "notification-2-period") {
                onSettingNotificationsPeriod(2, value);
            }

            else if (setting == "notification-3-period") {
                onSettingNotificationsPeriod(3, value);
            }

            else if (setting == "audio-react") {
                onSettingAudioReact(value);
            }

            else if (setting == "audio-group") {
                onSettingAudioGroup(value);
            }
        
            else if (setting == "midi-rx-ch") {
                onSettingMidiRxChannel(value);
            }
    
            else if (setting == "midi-first-note") {
                onSettingMidiFirstNote(value);
            }
    
            else if (setting == "midi-last-note") {
                onSettingMidiLastNote(value);
            }
    
            else if (setting == "midi-patch-to-preset") {
                onSettingMidiMapPatches(value);
            }
    
            else if (setting == "midi-preset-to-patch") {
                onSettingMidiMapPresets(value);
            }
    
            else if (setting == "midi-vol-to-bri") {
                onSettingMidiMapVolume(value);
            }
    
            else if (setting == "midi-bri-to-vol") {
                onSettingMidiMapBrightness(value);
            }
    
            else if (setting == "controlsync-active") {
                onSettingControlSyncActive(value);
            }

            else if (setting == "controlsync-group") {
                onSettingControlSyncGroup(value);
            }

            else if (setting == "controlsync-power") {
                onSettingControlSyncPower(value);
            }

            else if (setting == "controlsync-brightness") {
                onSettingControlSyncBrightness(value);
            }

            else if (setting == "controlsync-effect-mode") {
                onSettingControlSyncEffectMode(value);
            }            
            
            /***

            Only settable from the installation AP.

            else if (setting == "wifi-ssid") {
                onSettingWifiSsid(value);
            }

            else if (setting == "wifi-psk") {
                onSettingWifiPsk(value);
            }

            ***/

            /***

            All moved to onbouading AP UI

            else if (setting == "ntp-domain") {
            onSettingsNTPDomain(value);
            }

            else if (setting == "mqtt-address") {
                onSettingsMqttAddress(value);
            }

            else if (setting == "mqtt-port") {
                onSettingsMqttPort(value);
            }

            else if (setting == "mqtt-username") {
            onSettingsMqttUsername(value);
            }

            else if (setting == "mqtt-password") {
                onSettingsMqttPassword(value);
            }

            else if (setting == "led-count") {
                onSettingLedCount(value);
            }

            else if (setting == "led-density") {
                onSettingLedDensity(value);
            }

            else if (setting == "led-reverse") {
                onSettingLedReverse(value);
            }  

            else if (setting == "psu-volts") {
                onSettingPsuVolts(value);
            }

            else if (setting == "psu-milliamps") {
                onSettingPsuMilliamps(value);
            }

            else if (setting == "psu-managed") {
                onSettingManagePower(value);  
            }

            else if (setting == "fps") {
                onSettingFPS(value);
            }

            ***/

            else {
                console.error("ws: unrecognised setting '" +setting+ "'");
            }
        }

        else {
            console.log("ws: missing value for setting of '" +setting+ "'");
        }
    }

    else {
        console.error("ws: missing name of setting.");
    }
}

function onTimerMessage(args)
{
    if (args.length != 4) {
        console.log("ws: missing argument (state) from received timer command");
    }

    else
    {
        let timerNo = args[0];    // "1"
        let timeOfDay = args[1];  // "08:30"
        let daysOfWeek = args[2]; // "--M-WTF"
        let action = args[3];     // "-", "off", "on"

        setScheduledAction(timerNo, timeOfDay, daysOfWeek, action);
    }
}

function onStatusMessage(message)
{
    showStatusMessageBriefly(message, 5000);
}

function showStatusMessageBriefly(message, timeMs)
{
    showStatusMessage(message);

    setTimeout(
        () => { showStatusMessage(""); }
        , timeMs
    );
}

function showStatusMessage(message)
{
    let el = document.getElementById("status-line");

    if (el) {
        el.innerHTML = message;
    }
}

function onPowerMessage(args) 
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received power command");
    }

    else
    {
        state = args[0];

        if ((state == "on") || (state == "true")) {
            document.getElementById('powerSwitch').checked = true;
        } 

        else if ((state == "off") || (state == "false")) {
            document.getElementById('powerSwitch').checked = false;
        }

        else {
            console.log("ws: expected true/false for power command, but got " + state);
        }
    }
}

function isOn() {
    return document.getElementById('powerSwitch').checked;
}

function applyClass(elementId, className, on)
{
    let el = document.getElementById(elementId);

    if (el) {
        if (on) {
            el.classList.add(className);
        } else {
            el.classList.remove(className);
        }
    }
}

function onAirQualityMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received air-quality message");
    }

    else
    {
        value = args[0];
        var iaq = parseInt(value);

        if (isNaN(iaq)) {
            console.log("ws: temperature expected a specific air quality index, but got " + value);
        } 
        
        else {
            iaqGauge.refresh(iaq);
            updateIAQSummary(iaq);
        }
    }
}

function onAirQualityReadyMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received iaq ready message");
    }

    else
    {
        state = args[0];

        if ((state == "on") || (state == "true")) {
            // ??? Just wait for an IAQ value to come in, whereby the summary will get unpdaed anyway.
        } 

        else if ((state == "off") || (state == "false")) {
            showAirQualitySummary("Sensor is warming up...", "iaq-info-category-unavailable");
        }

        else {
            console.log("ws: expected true/false for iaq-ready command, but got " + state);
        }
    }
}

function updateIAQSummary(iaq)
{
    let text = "?";
    let textClass = "?";

    if (iaq <= 50) {
        text = "Good";
        textClass = "iaq-info-category-good";
    }

    else if (iaq <= 100) {
        text = "Moderate";
        textClass = "iaq-info-category-moderate";
    }

    else if (iaq <= 150) {
        text = "Sensitive";
        textClass = "iaq-info-category-sensitive";
    }

    else if (iaq <= 200) {
        text = "Unhealthy";
        textClass = "iaq-info-category-unhealthy";
    }

    else if (iaq <= 300) {
        text = "Very Unhealthy";
        textClass = "iaq-info-category-very-unhealthy";
    }

    else {
        text = "Hazardous";
        textClass = "iaq-info-category-hazardous";
    }

    showAirQualitySummary("Air Quality: " + text, textClass);
}

function showAirQualitySummary(summaryText, styleClassID)
{
    let elt = document.getElementById('iaq-summary-text');
    elt.innerHTML = summaryText;

    // let el = document.getElementById('iaq-summary-text-box');
    let el = document.getElementById('iaq-top-summary-text-box');
    el.classList.remove('iaq-info-category-good');
    el.classList.remove('iaq-info-category-moderate');
    el.classList.remove('iaq-info-category-sensitive');
    el.classList.remove('iaq-info-category-unhealthy');
    el.classList.remove('iaq-info-category-very-unhealthy');
    el.classList.remove('iaq-info-category-hazardous');
    el.classList.add(styleClassID);
}

function onTemperatureMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received temperature message");
    }

    else
    {
        let value = args[0];
        let tempC = parseFloat(value);

        if (isNaN(tempC)) {
            console.log("ws: temperature expected a specific temperature value, but got " + value);
        } 
        
        else {
            temperatureGauge.refresh(tempC);
        }
    }
}

function onHumidityMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received humidity message");
    }

    else
    {
        let value = args[0];
        let humidity = parseFloat(value);

        if (isNaN(humidity)) {
            console.log("ws: humidity - expected a specific humidity value, but got " + value);
        } 
        
        else {
            humidityGauge.refresh(humidity);
        }
    }
}

function onAirPressureMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received air pressure message");
    }

    else
    {
        let value = args[0];
        let pressure = parseInt(value);

        if (isNaN(pressure)) {
            console.log("ws: air pressure - expected a specific air pressure value, but got " + value);
        } 
        
        else {
            airPressureGauge.refresh(pressure);
        }
    }
}

function onTVOCMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received TVOC message");
    }

    else
    {
        let value = args[0];
        let tvoc = parseInt(value);

        if (isNaN(tvoc)) {
            console.log("ws: TVOC - expected a specific TVOC value, but got " + value);
        } 
        
        else {
            tvocGauge.refresh(tvoc);
        }
    }
}

function onCO2Message(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received CO2 message");
    }

    else
    {
        let value = args[0];
        let co2 = parseInt(value);

        if (isNaN(co2)) {
            console.log("ws: eCO2 - expected a specific CO2 value, but got " + value);
        } 
        
        else {
            eCo2Gauge.refresh(co2);
        }
    }
}

function onFanSpeedMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received fan-speed command");
    }

    else
    {
        let value = args[0];
        let speed = parseInt(value);

        if (isNaN(speed)) {
            console.log("ws: fan-speed - expected a specific speed value, but got " + value);
        } 
        
        else {
            let el = document.getElementById('fan-speed-slider');

            if (el) {
                el.value = speed;
            } else {
                console.log("ws: failed to find fan-speed-slider.");
            }

            fanSpeed = speed;
        }
    }
}

function setButtonSteadyGlow(buttonId, on) {
    applyClass(buttonId, "steady-glow", on);
}

function setButtonPulseGlow(buttonId, on) {
    applyClass(buttonId, "pulse-glow", on);
}

var maxRGBProperties = 6;
var maxHSVProperties = 3;
var maxIntProperties = 9;
var maxBoolProperties = 6;

var currentRGB = "#0000ff";

/*
function onSetLightRGBMessage(args)
{
    if (args.length == 0) {
        console.log("ws: missing argument (state) from received rgb command");
    }

    else 
    {
        rgbStr = args[0];
        currentRGB = rgbStr;
        var rgb = lightRGBPicker.color;
        rgb.set(rgbStr);
    }
}
*/

function capitalise(str) {
    return str[0].toUpperCase() + str.substring(1);
}

function parseIntOptions(optionsStr)
{
    var controlType = optionSlider;
    var controlOptions = [];

    if ((optionsStr.search(",") == 0) && (optionsStr.search(":") == 0) && (optionsStr.search("|") == 0)) {
        controlOptions.push(1);
        controlOptions.push(255);
        console.warn("Malformed option of '" + optionsStr + "', defaulting to slider with range '1,255'");
    }

    else if (optionsStr.includes(',')) {
        var limits = optionsStr.split(',');
        if (limits.length == 2) {
            var minVal = parseInt(limits[0]);
            var maxVal = parseInt(limits[1]);
            if (minVal != NaN) min = minVal;
            if (maxVal != NaN) max = maxVal;
            controlType = optionSlider;
            controlOptions.push(minVal);
            controlOptions.push(maxVal);
        }
    }

    else if (optionsStr.includes(':')) {
        var limits = optionsStr.split(':');
        if (limits.length == 2) {
            var minVal = parseInt(limits[0]);
            var maxVal = parseInt(limits[1]);
            if (minVal != NaN) min = minVal;
            if (maxVal != NaN) max = maxVal;
            controlType = optionSpinner;
            controlOptions.push(minVal);
            controlOptions.push(maxVal);
        }
    }

    else if (optionsStr.includes('|')) {
        console.log("Options of '" +optionsStr+ "'");
        var options = optionsStr.split('|');
        console.log("Number of options is " +options.length);
        controlType = optionDropdown;
        controlOptions = options;
    }

    return [controlType, controlOptions];
}

function onActualFrameRate(argStr) {
    setInnerText("fps-value", argStr);
}

function onHeapUsage(args)
{
    if (args.length < 2) {
        console.error("insufficient number of arguments in heap usage message.");
    }

    else
    {
        let totalHeap = args[0];
        let freeHeap = args[1];
        let usedHeap = totalHeap - freeHeap;

        setInnerText("heap-total-value", Math.floor(totalHeap / 1000));
        setInnerText("heap-used-value", Math.floor(usedHeap / 1000));
    }
}

function onDeviceDetectionMessage(args) 
{
    if (args.length < 2) {
        console.error("insufficient number of arguments in device detection message.");
    }

    else
    {
        var eventType = args[0];
        var ip = args[1];

        if (eventType == "online"){
            if (args.length < 3) {
                console.error("expected 4 (or min 3) arguments in add detected device message.");
            } else {
                var typeStr = args[2];
                var type = parseInt(typeStr);
                var name = ip; // If no name given then show the IP.
                if (args.length >= 4) {
                    name = args[3];
                    name = name.replaceAll('_', ' ');
                }
                addDevice(ip, type, name);
            }
        } 
        
        else if (eventType == "offline") {
            removeDevice(ip);
        } 
        
        else {
            console.error("unexpected device event of '" +eventType + "'.");
        }
    }
}


var showPreview = true;

function onSettingWifiSsid(value) {
    updateSettingNumeric("settings-wifi-ssid", value);
}

function onSettingWifiPsk(value) {
    updateSettingNumeric("settings-wifi-psk", value);
}

function onSettingsNTPDomain(value) {
    updateSettingNumeric("settings-ntp-domain", value);
}

function onSettingsMqttAddress(value) {
    updateSettingNumeric("settings-mqtt-address", value);
}

function onSettingsMqttPort(value) {
    updateSettingNumeric("settings-mqtt-port", value);
}

function onSettingsMqttUSername(value) {
    updateSettingNumeric("settings-mqtt-username", value);
}

function onSettingsMqttPassword(value) {
    updateSettingNumeric("settings-mqtt-password", value);
}

function updateSettingNumeric(elName, value) {
    var el = document.getElementById(elName);
    if (el) {
        el.value = value;
    } else {
        console.error("ws: missing element '" +elName+ "'");
    }
}

function updateSettingCheckbox(elName, value) {
    var on = (value == "on") || (value == "true") || (value == true);
    var el = document.getElementById(elName);
    if (el) {
        el.checked = on;
    } else {
        console.error("ws: missing element '" +elName+ "'");
    }
}

function initUI(tabIndex)
{
    var maxPresets = parseInt(maxPresetsStr);
    var userPresetsBase = parseInt(presetsBaseStr);

    var maxNotifications = parseInt(numNotificationsStr);
    var notificationsBase = parseInt(notificationsBaseStr);

    initEventListeners();
    initCollapsables();

    // Select the last selected tab. Allows the user to refresh the 
    // page but still be on the same tab that th ey last selected.

    document.getElementsByClassName('tab-link')[tabIndex].click();
}

function initEventListeners() 
{
    /* Done in HTML for better readability.
    powerEl = document.getElementById("powerSwitch");
    if (powerEl) powerEl.addEventListener("change", togglePower);
    */

    /* Done in HTML for better readability.
    brightnessEl = document.getElementById("brightnessSlider");
    if (brightnessEl) brightnessEl.addEventListener("change", changeBrightness);
    */

    /*

    prevEl = document.getElementById("previousEffectButton");
    if (prevEl) prevEl.addEventListener("click", previousEffect);

    nextEl = document.getElementById("nextEffectButton");
    if (prevEl) prevEl.addEventListener("click", nextEffect);
    
    */

    // document.getElementById("selectEffectDropdown").addEventListener("click", nextEffect);

    /* TODO: need a "Save" (or "Save & Restart") button to send Wifi {SSID, PSK} as an atomic tuple. */
    /* TODO: need a "Save" button to send PSU {V, mA} as an atomic tuple. */
    
    /// document.getElementById("numLEDsValue").addEventListener("change", setNumberOfLEDs);
}

function setPower(on) {
    if (on) {
        wsSend("power on");
    } else {
        wsSend("power off");
    }
}

function selectCurrentRGB() {
    // let picker = lightRGBPicker;
    let hex = lightRGBPicker.color.hexString;
    // let hex = getColorPickerRGB(picker);
    setLampRGB(hex);
}

var fanSpeed = 1;

function changeFanSpeed(speed) {
    wsSend("fan-speed " + speed);
    fanSpeed = speed;
}

function previousEffect() {
    wsSend("effect previous");
}

function nextEffect() {
    wsSend("effect next");
}

function selectPreset(presetNo) {
    wsSend("load-preset " + presetNo);
}

function selectEffect(effectNo) {
    wsSend("effect " + effectNo);
}

function selectEffectInstance(effectInstance) {
    wsSend("effect " + effectInstance);
}

function triggerAlert(alertNo) {
    wsSend("alert " + alertNo);
}

function addDevice(ip, type, name)
{
    if (allDevices.hasDevice(ip)) {
        removeDevice(ip);
    }

    allDevices.addDevice(new Device(ip, type, name));
    
    var newEl = createDeviceElement(ip, type, name);
    var parentEl = document.getElementById('devices-list');

    if (parentEl) {
        parentEl.appendChild(newEl);
    }

    if (allDevices.count() > 0) {
        setNetworkDevicesVisibility(true);
    } else {
        setNetworkDevicesVisibility(false);
    }
}

function removeDevice(ip)
{
    allDevices.removeDevice(ip);

    var deviceEl = document.getElementById('device-' + ip.replaceAll('.', '-'));

    if (deviceEl) {
        deviceEl.remove();
    }

    if (allDevices.count() > 0) {
        setNetworkDevicesVisibility(true);
    }
    
    else {
        if (selectedTabIndex == devicesTabIndex) {
            selectTab(controlsTabIndex, "controls-tab", "controls-tab-container");
        }
        setNetworkDevicesVisibility(false);
    }
}

function selectDevice(ip) {
    var url = "http://" + ip + "/ui/index.html";
    window.location.replace(url);
}

function createDeviceElement(ip, type, name)
{
    var button = document.createElement('button');

    button.classList.add('button');
    button.classList.add('devices-button');

    button.addEventListener('click', function() { selectDevice(ip); } );

    var typeImgEl = document.createElement('img');
    typeImgEl.src = createDeviceIconPathForType(type);
    button.appendChild(typeImgEl);

    var entryDivEl = document.createElement("div");
    entryDivEl.classList.add('devices-grid-item');
    entryDivEl.classList.add('device-entry-grid');

    var nameDivEl = document.createElement('div');
    nameDivEl.classList.add('device-entry-grid-item');
    nameDivEl.classList.add('devices-name-label');
    nameDivEl.innerHTML = name;

    var ipDivEl = document.createElement('div');
    ipDivEl.classList.add('device-entry-grid-item');
    ipDivEl.classList.add('device-detail-label');
    ipDivEl.innerHTML = "IP: " + ip;

    entryDivEl.appendChild(nameDivEl);
    entryDivEl.appendChild(ipDivEl);

    /***
    var detailsDivEl = document.createElement('div');
    detailsDivEl.classList.add('settings-grid-container');
    detailsDivEl.classList.add('devices-grid-container');

    ***/

    var parentDivEl = document.createElement('div');
    var id = 'device-' + ip.replaceAll('.', '-');
    parentDivEl.id = id;
    parentDivEl.classList.add('settings-grid-container');
    parentDivEl.classList.add('devices-grid-container');

    parentDivEl.appendChild(button);
    parentDivEl.appendChild(entryDivEl);

    return parentDivEl;
}

function createDeviceIconPathForType(type) 
{
    var iconName = "unknown-device";

    switch (type) {
        case 0x00: iconName = "computer"; break;
        case 0x01: iconName = "led-strip"; break;
        case 0x02: iconName = "lamp-outline"; break;
        case 0x21: iconName = "roller-shade"; break;
        case 0x41: iconName = "remote-tv"; break;
        case 0x60: iconName = "unknown"; break;  // TODO: temperature.
        case 0x61: iconName = "unknown"; break;  // TODO: humidity.
        case 0x62: iconName = "unknown"; break;  // TODO: temperature & humidity.
        case 0x63: iconName = "unknown"; break;  // TODO: occupancy.
        case 0x6f: iconName = "detector"; break; // multi/general sensor.
        case 0x70: iconName = "unknown"; break;  // Audio (sample etc).
        case 0x80: iconName = "gauge"; break;    // Monitoring/gauge.
        default: iconName = "unknown"; break;
    } 

    var path = "icons/" + iconName + ".png"; // TODO: svg?

    return path;
}

function populateDeviceList(devicesStr)
{
    var deviceArr = devicesStr.split("|");

    if (deviceArr.length > 0) 
    {
        var dropdown = document.getElementById('select-device-dropdown');

        if (dropdown)
        {
            if (dropdown.hasChildNodes())
            {
                // Adding/remving options may trigger the event listener
                // for a selection change, so we remove the listener
                // and then add it after we've finished modifying the dropdown.

                dropdown.removeEventListener("change", selectDevice);

                while (dropdown.hasChildNodes()) {
                    dropdown.removeChild(dropdown.firstChild);
                }
            
                var deviceOptions = dropdown.options;
                
                var selectedIndex = 0;
                var index = 0;

                deviceArr.forEach(function(deviceStr)
                {
                    var parts = deviceStr.split("~");

                    if (parts.length == 2)
                    {
                        var name =  parts[0];
                        name = name.replaceAll('_', ' ');

                        var ip = parts[1];

                        if (name.startsWith('@')) {
                            name = name.substring(1);
                            selectedIndex = index;
                        }

                        ++index;

                        deviceOptions.add(new Option(
                            name,
                            ip
                        ));

                    }
                });

                dropdown.selectedIndex = selectedIndex;

                dropdown.addEventListener("change", selectDevice);
            }

            else {
                console.error("Missing exemplar node in devices dropdown.");
            }
        }

        else {
            console.error("Could not find devices container node.");
        }
 }
}

/*
function setWifiCredentials(ssid, psk) {
    wsSend("wifi " + ssid + " " + psk);
}

function setNumberOfLEDs(noOfLEDs) {
    wsSend("leds " + noOfLEDs);
}

function setPowerSpecification(volts, milliAmps) {
    wsSend("psu " + volts + " " + milliAmps);
}
*/

/***
function onLampRGBChange(picker, propertyName) {
    // This callback responds to the ongoing selection of colour.
    var hex = getColorPickerRGB(picker);
    setLampRGB(hex);
}
***/

function onLampRGBButtonClicked() {
    colorPicker.on("input:end", onLampRGBColorPicked);
    rgbPickerPopupCallback = onLampRGBColorPicked;
    var rgb = lampRGBButton.style.backgroundColor;
    colorPicker.color.rgbString = rgb;
    colorPopup.style.display = "block";
}

function onLampRGBColorPicked(color) {
    var hex = colorPicker.color.hexString;
    lampRGBButton.style.backgroundColor = hex;
    setLampRGB(hex);
}

function onPropertyRGB1ButtonClicked() {
    colorPicker.on("input:end", onPropertyRGB1ColorPicked);
    rgbPickerPopupCallback = onPropertyRGB1ColorPicked;
    rgbButton = document.getElementById("rgb1-button");
    if (rgbButton) {
        var rgb = rgbButton.style.backgroundColor;
        colorPicker.color.rgbString = rgb;
        colorPopup.style.display = "block";
    }
}

function onPropertyRGB1ColorPicked(color) {
    rgbButton = document.getElementById("rgb1-button");
    if (rgbButton) {
        var hex = colorPicker.color.hexString;
        rgbButton.style.backgroundColor = hex;
        setEffectProperty(_currentEffectNo, "rgb1", hex);
    }
}

function onPropertyRGB2ButtonClicked() {
    colorPicker.on("input:end", onPropertyRGB2ColorPicked);
    rgbPickerPopupCallback = onPropertyRGB2ColorPicked;
    rgbButton = document.getElementById("rgb2-button");
    if (rgbButton) {
        var rgb = rgbButton.style.backgroundColor;
        colorPicker.color.rgbString = rgb;
        colorPopup.style.display = "block";
    }
}

function onPropertyRGB2ColorPicked(color) {
    rgbButton = document.getElementById("rgb2-button");
    if (rgbButton) {
        var hex = colorPicker.color.hexString;
        rgbButton.style.backgroundColor = hex;
        setEffectProperty(_currentEffectNo, "rgb2", hex);
    }
}

function onPropertyRGB3ButtonClicked() {
    colorPicker.on("input:end", onPropertyRGB3ColorPicked);
    rgbPickerPopupCallback = onPropertyRGB3ColorPicked;
    rgbButton = document.getElementById("rgb3-button");
    if (rgbButton) {
        var rgb = rgbButton.style.backgroundColor;
        colorPicker.color.rgbString = rgb;
        colorPopup.style.display = "block";
    }
}

function onPropertyRGB3ColorPicked(color) {
    rgbButton = document.getElementById("rgb3-button");
    if (rgbButton) {
        var hex = colorPicker.color.hexString;
        rgbButton.style.backgroundColor = hex;
        setEffectProperty(_currentEffectNo, "rgb3", hex);
    }
}

function onPropertyRGB4ButtonClicked() {
    colorPicker.on("input:end", onPropertyRGB4ColorPicked);
    rgbPickerPopupCallback = onPropertyRGB4ColorPicked;
    rgbButton = document.getElementById("rgb4-button");
    if (rgbButton) {
        var rgb = rgbButton.style.backgroundColor;
        colorPicker.color.rgbString = rgb;
        colorPopup.style.display = "block";
    }
}

function onPropertyRGB4ColorPicked(color) {
    rgbButton = document.getElementById("rgb4-button");
    if (rgbButton) {
        var hex = colorPicker.color.hexString;
        rgbButton.style.backgroundColor = hex;
        setEffectProperty(_currentEffectNo, "rgb4", hex);
    }
}

function onPropertyRGB5ButtonClicked() {
    colorPicker.on("input:end", onPropertyRGB5ColorPicked);
    rgbPickerPopupCallback = onPropertyRGB5ColorPicked;
    rgbButton = document.getElementById("rgb5-button");
    if (rgbButton) {
        var rgb = rgbButton.style.backgroundColor;
        colorPicker.color.rgbString = rgb;
        colorPopup.style.display = "block";
    }
}

function onPropertyRGB5ColorPicked(color) {
    rgbButton = document.getElementById("rgb5-button");
    if (rgbButton) {
        var hex = colorPicker.color.hexString;
        rgbButton.style.backgroundColor = hex;
        setEffectProperty(_currentEffectNo, "rgb5", hex);
    }
}

function onPropertyRGB6ButtonClicked() {
    colorPicker.on("input:end", onPropertyRGB6ColorPicked);
    rgbPickerPopupCallback = onPropertyRGB6ColorPicked;
    rgbButton = document.getElementById("rgb6-button");
    if (rgbButton) {
        var rgb = rgbButton.style.backgroundColor;
        colorPicker.color.rgbString = rgb;
        colorPopup.style.display = "block";
    }
}

function onPropertyRGB6ColorPicked(color) {
    rgbButton = document.getElementById("rgb6-button");
    if (rgbButton) {
        var hex = colorPicker.color.hexString;
        rgbButton.style.backgroundColor = hex;
        setEffectProperty(_currentEffectNo, "rgb6", hex);
    }
}

function onLampRGBUpdate(picker, propertyName)
{ 
    /***

    We choose NOT to update while the colour is being chosen as local 
    updates will conflict with closed-loop round-trip updates (which 
    we need to ensure all controllers/web-page instances are updated.)
    (Perhaps we can be clear and ignore updates being received iff
    until a local update has not been seen for a short while?. This
    allows other browser instances to be updated but not be contentious
    on the local browser. Maybe ignore if onfocus()?)

    if (trackColourDuringSelection) {
        now = new Date();
        var intervalSinceLastUpdate = now - lastColourSelectUpdateTime;
        if (intervalSinceLastUpdate >= lastColourUpdateInterval) {
            lastColourSelectUpdateTime = now;
            var hex = getColorPickerRGB(picker);
            setLampRGB(hex);
        }
    }

    ***/
}

function onRGBPropertyChange(picker, propertyName) {
    // This callback responds to the final selection of colour.
    var hex = getColorPickerRGB(picker);
    setEffectProperty(_currentEffectNo, propertyName, hex);
}

function onRGBPropertyUpdate(picker, propertyName)
{
    // This callback responds to the the ongoing browsing of colour prior to releasing the mouse/cursor
    // in the browser. Its continusously updated upto the point where the colour is finally selected.
    // Unfortunately, the underlying WS library can be a little fragile in these circumstances, so for now
    // we disable this code.
    // PERHAPS: we could perform this update, but limit it to only every m milliseconds. ???

    /***

    We choose NOT to update while the colour is being chosen as local 
    updates will conflict with closed-loop round-trip updates (which 
    we need to ensure all controllers/web-page instances are updated.)

    if (trackColourDuringSelection) {
        now = new Date();
        var intervalSinceLastUpdate = now - lastColourSelectUpdateTime;
        if (intervalSinceLastUpdate >= lastColourUpdateInterval) {
            lastColourSelectUpdateTime = now;
            var hex = getColorPickerRGB(picker);
            setEffectProperty(_currentEffectNo, propertyName, hex);
        }
    }

    ***/
}

function onHSVChange(picker, propertyName) {
    // This callback responds to the final selection of colour.
    var hex = getColorPickerHSV(picker);
    setEffectProperty(_currentEffectNo, propertyName, hex);
}

function onHSVUpdate(picker, propertyName) {
    // This callback responds to the the ongoing browsing of colour prior to releasing the mouse/cursor
    // in the browser. Its continusously updated upto the point where the colour is finally selected.
    // Unfortunately, the underlying WS library can be a little fragile in these circumstances, so for now
    // we disable this code.
    // PERHAPS: we could perform this update, but limit it to only every m milliseconds. ???
    if (trackColourDuringSelection) {
        now = new Date();
        var intervalSinceLastUpdate = now - lastColourSelectUpdateTime;
        if (intervalSinceLastUpdate >= lastColourUpdateInterval) {
            lastColourSelectUpdateTime = now;
            var hex = getColorPickerHSV(picker);
            setEffectProperty(_currentEffectNo, propertyName, hex);
        }
    }
}

function onIntSettingTrack(value, propertyName) {
    updateIntSliderValue(value, propertyName);
}

function onIntSettingChange(value, propertyName) {
    setEffectProperty(_currentEffectNo, propertyName, value);
}

function updateIntSliderValue(value, propertyName) {
    var valueElementID = propertyName + "-slider-value";
    updateSliderValue(value, valueElementID);
}

function updateSliderValue(value, sliderValueID) {
    var el = document.getElementById(sliderValueID);
    if (el) {
        el.innerHTML = value;
    }
}

function onBoolSettingChange(state, propertyName) {
    setEffectProperty(_currentEffectNo, propertyName, state ? "true" : "false");
}

function onReversalSettingChange(reversed) {
    setEffectProperty(_currentEffectNo, "reverse", reversed ? "on" : "off");
}

function onReflectionSettingChange(level) {
    setEffectProperty(_currentEffectNo, "reflection", level);
}

function onPixelWidthSettingChange(width) {
    setEffectProperty(_currentEffectNo, "pixel-width", width);
}

function onPixelGapSettingChange(gap) {
    setEffectProperty(_currentEffectNo, "pixel-gap", gap);
}

function onSavePreset() {
    var presetEl = document.getElementById("selectPresetDropdown");
    if (presetEl) {
        var presetNo = presetEl.value;
        var presetName = presetEl.value; // just the default. TODO: get the option label?
        var presetNameEl = document.getElementById("preset-name-textbox");
        if (presetNameEl) {
            presetName = presetNameEl.value;
        }
        savePreset(presetNo, presetName);
    }
}

function onLoadPreset() {
    var el = document.getElementById("selectPresetDropdown");
    var presetNo = el.value;
    loadPreset(presetNo);
}

function onSaveToClipboard() {
    saveToClipboard();
}

function onRestoreFromClipboard() {
    restoreFromClipboard();
}

function onResetEffectSettings() {
    resetEffectSettings();
}

function onExportEffect() 
{
    var el = document.getElementById("import-effect-textbox");

    if (el) {
        var markup = makeEffectInstanceMarkup();    
        el.value = markup;
    }

    if (typeof(navigator.clipboard) != "undefined") {
        navigator.clipboard.writeText(markup);
    } else {
        console.warn("Unable to copy to clipboard - user will have to do manually.");
    }

}

function onImportEffectInstance()
{
    var el = document.getElementById("import-effect-textbox");

    if (el) {
        importEffectInstance(el.value);
    }
}

function importEffectInstance(effectInstance) 
{
    var done = false;

    if (isValidEffectInstance(effectInstance)) {
        selectEffectInstance(effectInstance);
        done = true;
    }

    return done;
}

function isValidEffectInstance(expr) 
{
    var valid = false;

    try {
        parseEffectExpression(expr); // throws if invalid.
        valid = true;
    }

    catch (error) {
        console.error('Invalid effect instance expression: "' +expr+ '"');
    }

    return valid;
}

function parseEffectExpression(exprStr)
{
    var parsed = false;
                
    let pos = exprStr.indexOf(':');

    let classIDStr = exprStr.substr(0, pos);
    let propertiesStr = exprStr.substr(pos+1);

    var classID = parseInt(classIDStr);
    var properties = [];

    if (classID == NaN) {
        throw 'Invalid effect instance of "' +markup+ '" : invalid effect class ID';
    }
    
    let len = propertiesStr.length;
        
    if (len < 2) {
        throw 'Invalid effect instance of "' +markup+ '" : missing []';
    }

    var embraced = (propertiesStr[0] == '[') && (propertiesStr[len-1] == ']');

    if (!embraced) {
        throw 'Invalid effect instance of "' +markup+ '" : missing []';
    }

    parsed = true;

    propertiesStr = propertiesStr.substr(1, len-2);                
    var parts = propertiesStr.split(',');

    var index = 0;
    var numProperties = parts.length;

    while (parsed && (index < numProperties))
    {
        var part = parts[index++];
        var partParts = part.split(':');
                    
        if (partParts.length != 2) {
	        throw 'Invalid effect instance of "' +markup+ '" : malformed property';
        }

		var propertyID = partParts[0];
        var propertyValueStr = partParts[1];

        if (propertyID.length == 1)
        {
            propertyType= propertyID[0];

            switch (propertyType) 
            {
                case 'F':
                    if (propertyValueStr == 'T') {
                        propertyValue = true;
                    } else if (propertyValueStr == 'F') {
                        propertyValue = false;
                    } else {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid reverse property value of "' +propertyValueStr+ '"';
                    }
                    break;

                case 'K':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid reflection property value of "' +propertyValueStr+ '"';
                    }
                    break;

                case 'W':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid pixel width property value of "' +propertyValueStr+ '"';
                    }
                    break;

                case 'G':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid pixel gap property value of "' +propertyValueStr+ '"';
                    }
                    break;

                default:
                    throw 'Invalid effect instance of "' +markup+ '" : invalid property type of "' +propertyType+ '"';
                    break;
            }
        }

        else if (propertyID.length < 2) {
	        throw 'Invalid effect instance of "' +markup+ '" : malformed property';
        }

        else
        {
            var propertyType = propertyID[0];
            var propertyIndexStr = propertyID.substr(1);
            
            var propertyIndex = parseInt(propertyIndexStr);
                                
            if (propertyIndex == NaN) {
                throw 'Invalid effect instance of "' +markup+ '" : invalid property index';
            }

            var entry;
            var propertyValue = null;
                                    
            switch (propertyType) 
            {
                case 'R':
                    propertyValue = propertyValueStr;
                    break;
                                            
                case 'I':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid int property value of "' +propertyValueStr+ '"';
                    }
                    break;
                                            
                case 'B':
                    if (propertyValueStr == 'T') {
                        propertyValue = true;
                    } else if (propertyValueStr == 'F') {
                        propertyValue = false;
                    } else {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid bool property value of "' +propertyValueStr+ '"';
                    }
                    break;
                
                case 'F':
                    if (propertyValueStr == 'T') {
                        propertyValue = true;
                    } else if (propertyValueStr == 'F') {
                        propertyValue = false;
                    } else {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid reverse property value of "' +propertyValueStr+ '"';
                    }
                    break;

                case 'K':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid reflection property value of "' +propertyValueStr+ '"';
                    }
                    break;

                case 'W':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid pixel width property value of "' +propertyValueStr+ '"';
                    }
                    break;

                case 'G':
                    propertyValue = parseInt(propertyValueStr);
                    if (propertyValue == NaN) {
                        throw 'Invalid effect instance of "' +markup+ '" : invalid pixel gap property value of "' +propertyValueStr+ '"';
                    }
                    break;

                default:
                    throw 'Invalid effect instance of "' +markup+ '" : invalid property type of "' +propertyType+ '"';
                    break;
            }
            
            var property = [propertyType, propertyIndex, propertyValue];
            properties.push(property);
        }
    }

    return [classID, properties];
}

function makeEffectInstanceMarkup() 
{
    var markup = "";

    markup = getEffectClassId(); 
    markup = markup + ":[";

    first = true;

    for (let i=1; i <= maxRGBProperties; i++)
    {
        if (isRGBPropertyUsed(i))
        {
            if (first) {
                first = false;
            } else {
                markup = markup + ",";
            }

            markup = markup + "R" + i + ":" + colorAsHex(getRGBPropertyValue(i));
        }
    }

    for (let i=1; i <= maxHSVProperties; i++)
    {
        if (isHSVPropertyUsed(i))
        {
            if (first) {
                first = false;
            } else {
                markup = markup + ",";
            }

            markup = markup + "H" + i + ":" + colorAsHex(getHSVPropertyValue(i));
        }
    }

    for (let i=1; i <= maxIntProperties; i++)
    {
        if (isIntPropertyUsed(i))
        {
            if (first) {
                first = false;
            } else {
                markup = markup + ",";
            }

            markup = markup + "I" + i + ":" + getIntPropertyValue(i);
        }
    }

    for (let i=1; i <= maxBoolProperties; i++)
    {
        if (isBoolPropertyUsed(i))
        {
            if (first) {
                first = false;
            } else {
                markup = markup + ",";
            }

            markup = markup + "I" + i + ":" + (getBoolPropertyValue(i) ? "T" : "F");
        }
    }

    if (isReversePropertyUsed())
    {
        if (first) {
            first = false;
        } else {
            markup = markup + ",";
        }

        markup = markup + "F:" + (getBoolPropertyValueByName("reverse-checkbox") ? "T" : "F");
    }

    if (isReflectionPropertyUsed())
    {
        if (first) {
            first = false;
        } else {
            markup = markup + ",";
        }

        markup = markup + "K:" + getIntPropertyValueByName("reflection-dropdown");
    }

    if (isPixelWidthPropertyUsed())
    {
        if (first) {
            first = false;
        } else {
            markup = markup + ",";
        }

        markup = markup + "W:" + getIntPropertyValueByName("pixel-width-slider");
    }
    
    if (isPixelGapPropertyUsed())
    {
        if (first) {
            first = false;
        } else {
            markup = markup + ",";
        }

        markup = markup + "G:" + getIntPropertyValueByName("pixel-gap-slider");
    }

    markup = markup + "]";

    return markup;
}

function colorAsHex(rgbStr) {
    var parts = rgbStr.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
    delete(parts[0]);
    for (var i = 1; i <= 3; ++i) {
        parts[i] = parseInt(parts[i]).toString(16);
        if (parts[i].length == 1) parts[i] = '0' + parts[i];
    }
    return parts.join('');
} 

function isRGBPropertyUsed(n) {
    return !isCollapsedPropertyControl("rgb", n, "picker");
}

function getRGBPropertyValue(n) {
    var value = "000000";
    var elName = "rgb" + n + "-button";
    var el = document.getElementById(elName);
    if (el) {
        value = el.style.backgroundColor;
    }
    return value;
}

function isHSVPropertyUsed(n) {
    return !isCollapsedPropertyControl("hsv", n, "picker");
}

function getHSVPropertyValue(n) {
    var value = "000000";
    var elName = "hsv" + n + "-button";
    var el = document.getElementById(elName);
    if (el) {
        value = el.style.backgroundColor;
    }
    return value;
}

function isIntPropertyUsed(n) {
    return !isCollapsedPropertyControl("int", n, "slider") || !isCollapsedPropertyControl("int", n, "spinner") || !isCollapsedPropertyControl("int", n, "dropdown");
}

function getIntPropertyValue(n)
{
    var value = 0;
    var elName = "";

    if (!isCollapsedPropertyControl("int", n, "slider")) {
        elName = "int" + n + "-slider";
    } else if (!isCollapsedPropertyControl("int", n, "spinner")) {
        elName = "int" + n + "-spinner";
    } else if (!isCollapsedPropertyControl("int", n, "dropdown")) {
        elName = "int" + n + "-dropdown";
    }
    
    if (elName != "") {
        var el = document.getElementById(elName);
        if (el) {
            value = el.value;
        }
    }

    return value;
}

function getEffectClassId() {
    var classId = 0;
    var elName = "selectEffectDropdown";
    var el = document.getElementById(elName);
    if (el) {
        classId = el.value;
    }
    return classId;
}

function isBoolPropertyUsed(n) {
    return !isCollapsedPropertyControl("bool", n, "checkbox");
}

function isReversePropertyUsed() {
    return !isCollapsedPropertyControlByName("reverse-checkbox-container");
}

function isReflectionPropertyUsed() {
    return !isCollapsedPropertyControlByName("reflection-dropdown-container");
}

function isPixelWidthPropertyUsed() {
    return !isCollapsedPropertyControlByName("pixel-width-slider-container");
}

function isPixelGapPropertyUsed() {
    return !isCollapsedPropertyControlByName("pixel-gap-slider-container");
}

function getBoolPropertyValue(n) {
    var elName = "bool" + n + "-checkbox";
    return getBoolPropertyValueByName(elName);
}

function getBoolPropertyValueByName(elName) {
    var value = false;
    var el = document.getElementById(elName);
    if (el) {
        value = el.checked;
    }
    return value;
}

function getIntPropertyValueByName(elName) {
    var value = false;
    var el = document.getElementById(elName);
    if (el) {
        value = el.value;
    }
    return value;
}

function getColorPickerRGB(picker) {
    return picker.toHEXString();
}

function getColorPickerHSV(picker) {
    var h = picker.channel('H'); // 0 .. 360
    var s = picker.channel('S'); // 0 .. 100
    var v = picker.channel('V'); // 0 .. 100
    return "#" + byteToHex(h) + byteToHex(s) + byteToHex(v);
}

function setLampRGB(hex) {
    var message = "rgb " + hex;
    wsSend(message);
}

function setEffectProperty(effectNo, propertyName, propertyValue) {
    var message = "effect-setting " + effectNo.toString() + " " + propertyName.toLowerCase() + " " + propertyValue;
    wsSend(message);
}

function savePreset(presetNo, presetName) { // -ve value implies factory preset, +ve means user preset.
    var encodedName = presetName.trim().replaceAll(' ', '_');
    // var message = "save-preset " + presetNo.toString() + " " + encodedName;
    var message = "save-preset " + presetNo + " " + encodedName;
    wsSend(message);
}

function selectCoolWhite() {
    if (true) {
        var message = "white cool";
        wsSend(message);
    } else {
        loadPreset(-1);
    }
}

function selectNeutralWhite() {
    if (true) {
        var message = "white neutral";
        wsSend(message);
    } else {
        loadPreset(-1);
    }
}

function selectWarmWhite() {
    if (true) {
        var message = "white warm";
        wsSend(message);
    } else {
        loadPreset(-2)
    }
}

/*
function selectCurrentRGB() {
    if (true) { 
        let message = "rgb " + currentRGB;
        wsSend(message);
    } else {
        loadPreset(-3)
    }
}
*/

function selectRemoteMode() {
    if (true) {
        var message = "effect 3"; // Effect class #3 is the E1.31 effect.
        wsSend(message);
    } else {
        loadPreset(-4);
    }
}

function loadPreset(presetNo) { // -ve value implies factory preset, +ve means user preset.
    var message = "load-preset " + presetNo.toString();
    wsSend(message);
}

function saveToClipboard() {
    wsSend("clipboard save");
}

function restoreFromClipboard() {
    wsSend("clipboard load");
}

function resetEffectSettings() {
    wsSend("effect-reset");
}

function onChangeAudioSetting(settingName, value) {
    wsSend("audio-setting " +settingName+ " " +value);
    // Update the label to the slider's right, that shows the numeric value.
    var elName = "audio" + capitalise(settingName) + "Value";
    updateSettingNumeric(elName, value);
}

function onChangeAudioChannelSetting(settingName, value) {
    wsSend("audio-setting " +settingName+ " " +value);
}

function saveConstrainedSetting(settingName, element)
{
    const currentValue = parseInt(element.value);

    if (currentValue < element.min) {
        element.value = element.min;
    }

    if (currentValue > element.max) {
        element.value = element.max;
    }

    saveSetting(settingName, element.value);
}

function saveSetting(settingName, value) {
    wsSend("setting "+settingName+ " " +value);
}

function toggleSetting(settingName) {
    wsSend("setting " +settingName+ " toggle");
}

function onSaveSettingsRequiringReboot()
{
    var commandDelimiter = "~";

    var wholeMessage = "";

    var ledCountEl = document.getElementById("settings-led-count");
    var ledCount = ledCountEl.value;
    var message = "setting led-count " + ledCount;
    if (wholeMessage.length > 0) wholeMessage += commandDelimiter;
    wholeMessage += message;

    var ledReverseEl = document.getElementById("settings-led-reverse");
    var reverse = (ledReverseEl.value != 0);
    var message = "setting led-reverse " + (reverse ? "on" : "off");
    if (wholeMessage.length > 0) wholeMessage += commandDelimiter;
    wholeMessage += message;

    var message = "reboot";
    if (wholeMessage.length > 0) wholeMessage += commandDelimiter;
    wholeMessage += message;
    
    wsSend(wholeMessage);
}

function wsSend(message) {
    console.log("ws: tx '" +message+ "'")
    websocket.send(message);
}

function setInnerText(elName, text) {
    let el = document.getElementById(elName);
    if (el) {
        el.innerText = text;
    } else {
        console.error("ws: missing element '" +elName+ "'");
    }
}

function hexToRgb(hex) {
    let result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
}

function byteToHex(val) {
    var hex = val.toString(16);
    return (hex.length == 1) ? "0" + hex : hex;
}

function getInitialTabByIndex()
{
    var override = false;

    var defaultTabIndex = 0;
    var tabIndex = defaultTabIndex;

    var params = new URLSearchParams(window.location.search);

    if (params.has("tab")) {
        indexStr = params.get("tab");
        index = parseInt(indexStr);
        if (!isNaN(index)) {
            tabIndex = index;
            override = true;
        }
    }

    if (!override) {
        tabIndex = getCookie("selectedTabIndex", defaultTabIndex);
    }

    if (tabIndex == devicesTabIndex) { /* The devices tab */
        tabIndex = controlsTabIndex; /* The primary control tab */
    }

    return tabIndex;
}

/*
function initParamValuesTable() {
    params = new URLSearchParams(window.location.search);
    showParams = false;
    if (params.has("showparams")) {
        showStr = params.get("showparams");
        showStr = showStr.trim();
        showStr = showStr.toLowerCase();
        showParams = (showStr == "true");
    }
    element = document.getElementById("paramvaluesdiv");
    element.style.visibility = (showParams ? "visible" : "collapse");
}
*/

function setCookie(cookieName, value) {
    document.cookie = cookieName + "=" + value;
}

function getCookie(cookieName, defaultValue) {
    let cookies = getCookies();
    let value = (cookies.has(cookieName)) ? cookies.get(cookieName) : defaultValue;
    console.info("Cookie '" + cookieName + "' is '" + value + "'");
    return value;
}

function getCookies()
{
    var cookies = new Map();

    allCookiesStr = document.cookie;
    allCookies = allCookiesStr.split(';');

    console.log("Cookies are: '" +allCookiesStr+ "'");

    if (allCookiesStr.length > 0) {
        allCookies.forEach(cookie => {
        cookie = cookie.trim();
        if (cookie.length > 0) {
            // console.log("Cookie is: '" +cookie+ "'");
            cookieParts = cookie.split('=');
            var name = cookieParts[0].trim();
            var value = cookieParts[1].trim();
            cookies.set(name, value);
        }
    });
}

    return cookies;
}

function setOnlineState(state)
{
    bannerRight = document.getElementById("banner-icon-container-right");
    icon = connectivityIcon(state);

    clonedIcon = icon.cloneNode(true);
    clonedIcon.removeAttribute("id");

    while (bannerRight.firstChild) {
        bannerRight.removeChild(bannerRight.firstChild);
    }

    bannerRight.appendChild(clonedIcon);
}

function connectivityIcon(state)
{
    var icon;

    if (state) {
        icon = document.getElementById("online-icon");
    } else {
        icon = document.getElementById("offline-icon");
    }

    return icon;
}

function drawAudioHistograms(volumes, frequencyData, fullDraw) {
    drawVolumeHistogram(volumes, fullDraw);
    drawFrequencyHistogram(frequencyData, fullDraw);
}

function drawVolumeHistogram(volumes, fullDraw)
{
    var canvas = document.getElementById("volumeCanvas");

    var workingCanvas = document.createElement('canvas');
    workingCanvas.width = canvas.width;
    workingCanvas.height = canvas.height;

    var marginH = 5;
    var marginV = 5;    
    var spacing = 4;
    var unlitRGB = volumeUnlitRGB;
	var litRGB = volumeLitRGB;
	var backRGB = audioHistogramBackground;

    drawHistogram(workingCanvas, volumes, marginH, marginV, spacing, unlitRGB, litRGB, backRGB, fullDraw);
    
    var ctx = canvas.getContext('2d');
    ctx.save(); // Freeze screen updates.
    ctx.drawImage(workingCanvas, 0, 0);
    ctx.restore(); // Unfreeze screen updates, showing the result.
}

function drawFrequencyHistogram(frequencyData, fullDraw)
{
    var canvas = document.getElementById("frequencyCanvas");
    
    var workingCanvas = document.createElement('canvas');
    workingCanvas.width = canvas.width;
    workingCanvas.height = canvas.height;

    var marginH = 5;
    var marginV = 5;    
    var spacing = 4;
	var unlitRGB = frequenciesUnlitRGB;
	var litRGB = frequenciesLitRGB;  
    var backRGB = audioHistogramBackground;

    drawHistogram(workingCanvas, frequencyData, marginH, marginV, spacing, unlitRGB, litRGB, backRGB, fullDraw);

    var ctx = canvas.getContext('2d');
    ctx.save(); // Freeze screen updates.
    ctx.drawImage(workingCanvas, 0, 0);
    ctx.restore(); // Unfreeze screen updates, showing the result.
}

function drawHistogram(canvas, dataArr, marginH, marginV, spacing, unlitRGB, litRGB, backRGB, drawBackground)
{
    var totalW = canvas.width;
    var totalH = canvas.height;

    var n = dataArr.length;

    var totalVoidW = marginH * 2 + spacing * (n - 1);

    var barW = (totalW - totalVoidW) / n;
    
    var totalVoidH = 2 * marginV;
    var maxBarH = totalH - totalVoidH;

    var barScaleH = maxBarH / 100.0; 

    var ctx = canvas.getContext("2d");

    /*
    Flip coordinate system so B,L is 0, 0
    ctx.translate(0, totalH);
    ctx.scale(1, -1);
    */

    if (drawBackground) {
        ctx.fillStyle = backRGB;
        ctx.fillRect(0, 0, totalW, totalH);
    }

    var x = marginH;
    var y = marginV;

    ctx.fillStyle = unlitRGB;

    for (let i in dataArr) {
        var w = barW;
        var h = 100 * barScaleH;
        ctx.fillRect(x, y, w, h);
        x += barW + spacing;
    }

    var x = marginH;
    var y = marginV;

    ctx.fillStyle = litRGB;

    for (let i in dataArr) {
        var w = barW;
        var h = dataArr[i] * barScaleH;
        ctx.fillRect(x, y, w, h);
        x += barW + spacing;
    }    
}

function initCollapsables()
{
    var collapsibles = document.getElementsByClassName("collapsible");

    for (let i = 0; i < collapsibles.length; i++)
    {
        var collapsible = collapsibles[i];
        //console.info("For collapsible with id '" + collapsible.id + " classes are '" + collapsible.classList + "'");

        collapsible.addEventListener("click", function() 
        {
            let origin = event.target;
            //console.info("Click event from element withid '" + origin.id + "' - classes are '" + origin.classList + "'");

            // The help icons are on top of the collapsible, meaning that when clicked
            // the collapsible *also* gets a click event. Which we don't want.
            // So we look to see what has initiated the click.

            if (!isOfClass(origin, "help-icon")) //  as might be (e.g. other clickable element on top being selected)
            {
                var openIndicators = this.getElementsByClassName("openIndicator");
                var closedIndicators = this.getElementsByClassName("closedIndicator");

                this.classList.toggle("active");
                var content = this.nextElementSibling;

                if (content.style.display === "block")
                {
                    content.style.display = "none";

                    if (openIndicators.length > 0) {
                        openIndicators[0].style.display="none";
                    }

                    if (closedIndicators.length > 0) {
                        closedIndicators[0].style.display="inline-block";
                    }
                }
            
                else
                {
                    content.style.display = "block";

                    if (openIndicators.length > 0) {
                        openIndicators[0].style.display="inline-block";
                    }

                    if (closedIndicators.length > 0) {
                        closedIndicators[0].style.display="none";
                    }
                }
            }
        });

        var openIndicators = collapsible.getElementsByClassName("openIndicator");

        if (openIndicators.length > 0) {
            var openIndicator = openIndicators[0];
            openIndicator.style.display="none";
        }

        var closedIndicators = collapsible.getElementsByClassName("closedIndicator");

        if (closedIndicators.length > 0) {
            var closedIndicator = closedIndicators[0];
            closedIndicator.style.display="inline-block";
        }

        collapsible.nextElementSibling.style.display = "none"; /* initially all collapsed */
    }
}

function sortSelect(selElem)
{
    var tmpAry = new Array();

    for (var i=0;i<selElem.options.length;i++) {
        tmpAry[i] = new Array();
        tmpAry[i][0] = selElem.options[i].text;
        tmpAry[i][1] = selElem.options[i].value;
    }

    tmpAry.sort();

    while (selElem.options.length > 0) {
        selElem.options[0] = null;
    }

    for (var i=0;i<tmpAry.length;i++) {
        var op = new Option(tmpAry[i][0], tmpAry[i][1]);
        selElem.options[i] = op;
    }

    return;
}

function sortEffectNameControlOptions() {
    var el = document.getElementById("selectEffectDropdown");
    if (el) {
        sortSelect(el);
    }
}

var lightRGBPicker = null;

var colorPicker = null;
var colorPopup = null;
var colorPopupSpan  = null; 
var rgbPickerPopupCallback = null;

function initialiseLightColourSelector()
{
    lightRGBPicker = new iro.ColorPicker('#light-rgb-picker', {
        borderWidth: 2,
        width: 190,
        wheelLightness: false,
        layout: [
        {
            component: iro.ui.Wheel,
            options: {
                borderColor: '#A0A0A0'
            }
        },
        {
            component: iro.ui.Slider,
            options: {
                borderColor: '#A0A0A0'
            }
        }
        ]
        /*
        layout: [
        {
            // component: iro.ui.Wheel,
            component: iro.ui.Box,
            options: {
                borderColor: '#fff',
                borderWidth: 1
            }
        },
        {
            component: iro.ui.Slider,
            options: {
                sliderType: 'hue',
                sliderSize: 30,
                borderWidth: 1,
                // sliderDirection: 'vertical',
                sliderShape: 'box'
            },
        }
        ]
        */
    });

    lightRGBPicker.on(["input:end"], function(colour) {
        setLampRGB(colour.hexString);
    });
}

function initialiseColourSelectorPopup()
{
    colorPicker = new iro.ColorPicker("#color-picker", {
        // Set the size of the color picker
        width: 200,
        // Stop the wheel from fading when luminosity is decreased.
        wheelLightness: false,
        layout: [
        {
            component: iro.ui.Wheel,
            options: {
                borderColor: '#808080',
                borderWidth: 1
            }
        },
        {
            component: iro.ui.Slider,
            options: {
                borderColor: '#808080',
                borderWidth: 1
            }
        }
        ]
    });


    // Get the modal.

    colorPopup = document.getElementById("color-picker-popup");
            
    // Get the <span> element that closes the modal.

    colorPopupSpan = document.getElementById("color-popup-close");

    // When the user clicks on <span> (x), close the modal.

    colorPopupSpan.onclick = function(event) {
        colorPopup.style.display = "none";
        if (rgbPickerPopupCallback) {
            colorPicker.off("input:end", rgbPickerPopupCallback);
            rgbPickerPopupCallback = null;
        }
    }
}

function initialisePage() 
{
    selectedTabIndex = getInitialTabByIndex();
    initUI(selectedTabIndex);   

    setOnlineState(false); // websocket connection status.

    // Ensure UI is initialised before hooking up web socket so that
    // any callbacks have a fully-formed UI to update.

    initWebSocket();

    /* initBubbleSliders(); */
    /* initTimePickers(); */

    setNetworkDevicesVisibility(false);
}

/*
function initTimePickers()
{
    tp.attach({
        target: document.getElementById("timer-1-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(1)
    });

    tp.attach({
        target: document.getElementById("timer-2-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(2)
    });

    tp.attach({
        target: document.getElementById("timer-3-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(3)
    });

    tp.attach({
        target: document.getElementById("timer-4-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(4)
    });

    tp.attach({
        target: document.getElementById("timer-5-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(5)
    });

    tp.attach({
        target: document.getElementById("timer-6-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(6)
    });

    tp.attach({
        target: document.getElementById("timer-7-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(7)
    });

    tp.attach({
        target: document.getElementById("timer-8-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(8)
    });

    tp.attach({
        target: document.getElementById("timer-9-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(9)
    });

    tp.attach({
        target: document.getElementById("timer-10-time"),
        "24": true, // 24 hours
        after : time => onTimerParamChange(10)
    });
}
***/

/***
function setTimerParam(timerNo)
{
    var elNameTime = "timer-" + timerNo + "-time";
    var elTime = document.getElementById(elNameTime);
    var time = elTime.value;
    setTimerTime(timerNo, time);
}
***/

function onTimerParamChange(timerNo) {
    var definition = getScheduledAction(timerNo);
    var message = "timer " + definition;
    wsSend(message);    
}

function setScheduledAction(timerNo, timeOfDay, daysOfWeek, action)
{
    setScheduledActionTimeOfDay(timerNo, timeOfDay);
    setScheduledActionDaysOfWeek(timerNo, daysOfWeek);
    setScheduledActionCommand(timerNo, action);
}

function setScheduledActionTimeOfDay(timerNo, timeOfDay)
{
    var elName = "timer-" + timerNo + "-time";
    var el = document.getElementById(elName);

    if (!el) {
        console.error("Missing time element for scheduled action " + timerNo + " (so not setting).");
    }

    else {
        el.value = timeOfDay;
    }
}

function setScheduledActionDaysOfWeek(timerNo, daysOfWeek)
{
    setScheduledActionDayOfWeek(timerNo, "sat", (daysOfWeek[0] == "S"));
    setScheduledActionDayOfWeek(timerNo, "sun", (daysOfWeek[1] == "S"));
    setScheduledActionDayOfWeek(timerNo, "mon", (daysOfWeek[2] == "M"));
    setScheduledActionDayOfWeek(timerNo, "tue", (daysOfWeek[3] == "T"));
    setScheduledActionDayOfWeek(timerNo, "wed", (daysOfWeek[4] == "W"));
    setScheduledActionDayOfWeek(timerNo, "thu", (daysOfWeek[5] == "T"));
    setScheduledActionDayOfWeek(timerNo, "fri", (daysOfWeek[6] == "F"));
}

function setScheduledActionDayOfWeek(timerNo, dow, checked)
{
    var elName = "timer-" + timerNo +"-day-" + dow;
    var el = document.getElementById(elName);

    if (!el) {
        console.error("Missing day-of-week (" + dow + ") element (" + elName + ")  for scheduled action " + timerNo + " (so not setting).");
    }
    
    else {
        el.checked = checked;
    }
}

function setScheduledActionCommand(timerNo, action)
{
    var elName = "timer-" + timerNo + "-action";
    var el = document.getElementById(elName);

    if (!el) {
        console.error("Missing action element for scheduled action " + timerNo + " (so not setting).");
    }
    
    else
    {
        el.value = action;

        /*

        if (action == "-") {
            el.value = "-";
        }

        else if (action == "on") {
            el.value = "on";
        }

        else if (action == "off") {
            el.value = "off";
        }

        else if (action.startsWith("*")) {
            let brightnessStr = "*" + action.substring(1);
            el.value = brightnessStr;
        }

        else if (action.startsWith("#")) {
            let presetNoStr = "#" + action.substring(1);
            el.value = presetNoStr;
        }

        else {
            console.error("unrecognised action of '" + action + "'");
        }
        
        */
    }
}

function getScheduledAction(timerNo)
{
    var time = getScheduledActionTimeOfDay(timerNo);
    var daysOfWeek = getScheduledActionDaysOfWeek(timerNo);
    var action = getScheduledActionCommand(timerNo);

    return timerNo + " " + time + " " + daysOfWeek + " " + action;
}

function getScheduledActionTimeOfDay(timerNo)
{
    var elName = "timer-" + timerNo + "-time";
    var el = document.getElementById(elName);

    if (!el) {
        console.error("Missing time element for scheduled action " + timerNo + " (so defaulting).");
    }

    return el ? el.value : "00:00";
}

function getScheduledActionDaysOfWeek(timerNo)
{
    var sat = getScheduledActionDayOfWeek(timerNo, "sat");
    var sun = getScheduledActionDayOfWeek(timerNo, "sun");
    var mon = getScheduledActionDayOfWeek(timerNo, "mon");
    var tue = getScheduledActionDayOfWeek(timerNo, "tue");
    var wed = getScheduledActionDayOfWeek(timerNo, "wed");
    var thu = getScheduledActionDayOfWeek(timerNo, "thu");
    var fri = getScheduledActionDayOfWeek(timerNo, "fri");

    var daysOfWeek = "";

    daysOfWeek = daysOfWeek + (sat ? "S" : "-");
    daysOfWeek = daysOfWeek + (sun ? "S" : "-");
    daysOfWeek = daysOfWeek + (mon ? "M" : "-");
    daysOfWeek = daysOfWeek + (tue ? "T" : "-");
    daysOfWeek = daysOfWeek + (wed ? "W" : "-");
    daysOfWeek = daysOfWeek + (thu ? "T" : "-");
    daysOfWeek = daysOfWeek + (fri ? "F" : "-");

    return daysOfWeek;
}

function getScheduledActionDayOfWeek(timerNo, dow)
{
    var elName = "timer-" + timerNo +"-day-" + dow;
    var el = document.getElementById(elName);

    if (!el) {
        console.error("Missing day-of-week element (" + dow + ") for scheduled action " + timerNo + " (so defaulting).");
    }
    
    return el ? el.checked : false;
}

function getScheduledActionCommand(timerNo)
{
    let action = "-";

    var elName = "timer-" + timerNo + "-action";
    var el = document.getElementById(elName);

    if (!el) {
        console.error("Missing action element for scheduled action " + timerNo + " (so defaulting).");
    }
    
    else {
        action = el.value;
    }

    return action;
}

function setNetworkDevicesVisibility(isVisible)
{
    var el = document.getElementById("devices-tab");

    if (el) {
        el.style.visibility = isVisible ? "visible" : "hidden";
    }
}

function initHelpPopup()
{
    modal = document.getElementById("help-popup");
    modalCloseBtn = document.getElementById("help-popup-close");
    modalText = document.getElementById("help-popup-text");

    if (!modal) {
        console.error("Cannot find help popup modal element.");
    }
    
    if (!modalCloseBtn) {
        console.error("Cannot find help popup modal close button element.");
    }

    if (!modalText) {
        console.error("Cannot find help popup modal text element.");
    }    

    modalCloseBtn.addEventListener("click", hideModal);
}


function loadTheme(theme)
{
    var el = document.getElementById('theme');

    if (el) {
        let themePath = "themes/" + theme + ".css";
        // themePath = themePath + "?nocache=" + Date.now(); // Prevents browser caching.
        console.info("Loading theme from '" + themePath + "'");
        el.href = themePath;
        // Do a gratuitous change to the style sheet in order to prompt
        // an immediate availability of the new root variable values.
        el.style.setProperty("--dummy", "/");
        // ...nice theory, but didn't work. :(
        el.classList.add("dummyClass");

        /*

        Should work - but no update() method.
        Need later version of JustGage. TODO: get it!
        https://github.com/toorshia/justgage
        v 1.6.1. Also has a min.js variant!

        */
    }
}

function selectTheme(theme)
{
    console.log("Setting theme to: " + theme);

    loadTheme(theme);
    setCookie("theme", theme);

    /*
    refreshPage();
    */

    /* Need to reload root vars as they will have changed. */

    /*
    console.log("Reading root vars for theme " + theme + "...");
    readRootVars();
    console.log("Root vars for theme " + theme + " are:");
    logRootVars();

    console.log("###### New font color is " + gaugeValueColor);
    iaqGauge.valueFontColor = gaugeValueColor;
    iaqGauge.update('valueFontColor', gaugeValueColor);
    */

    /* Can't do this here - too soon as load of new theme is performed async by the browser.
   onThemeChanged();
    */

    /*
    The link for 'theme' seems to not trigger the onload event when the href is changed.
    This can be due to different browsers having different behavioiur.
    So lets try deleting the link and re-adding it.
    */

    let themeEl = document.getElementById('theme');
    let clonedEl = themeEl.cloneNode();

    let newHref = 'themes/' + theme + '.css';
    clonedEl.href = newHref;
    clonedEl.type = 'text/css';
    clonedEl.onload = onThemeChanged;

    themeEl.parentNode.replaceChild(clonedEl, themeEl);
}

function onThemeChanged()
{
    console.log("Theme loaded.");

    readRootVars();
    logRootVars();

    /* This just adds some duplicates. Not even all. Weird.
    console.log("Recreating gauges.");
    recreateGauges();
    */

    // iaqGauge.config.valueFontColor = gaugeValueColor;
    // iaqGauge.valueFontColor = gaugeValueColor;
    
    /* Mmm, this just doubles up the number. Need to remove the old ones first...
    Note: the new ones DO HAVE THE CORRECT THEME APPLIED! Yay!
    makeGauges();
    */
}

function onLoad(event)
{
    console.log("Executing onLoad()");

    // readRootVars();

    initHelpPopup();
    initialisePage();
    initAudioRendering();

    let theme = "dark";

    /*

    theme = getCookie("theme");

    if (theme == undefined) {
        console.info("No theme cookie was found.");
        theme = "dark";
    }
 
    */

    console.info("Starting with theme '" + theme + "'");

    readRootVars();
    logRootVars();

    /*

    loadTheme(theme);

    let el = document.getElementById("settings-theme");
    if (el) {
        el.value = theme;
    }

    */

    let rootEl = document.getElementById("content");
    let loadingEl = document.getElementById("loading-screen");

    // loadingEl.classList.add("hidden");
    // loadingEl.style.visibility = collapsed;
    // loadingEl.remove();

    // Initially, just a loading screen is should, but when
    // everything is loaded and dynamically massaged, we
    // reveal the actual control page.

    loadingEl.style.display = "none";
    rootEl.classList.remove("hidden");

    // When the user clicks anywhere outside of the modal, close it.

    window.onclick = function(event)
    {
        if ((event.target == colorPopup)) {
            colorPopup.style.display = "none";
            if (rgbPickerPopupCallback) {
                colorPicker.off("input:end", rgbPickerPopupCallback);
                rgbPickerPopupCallback = null;
            }
        }

        else if (event.target == modal) {
            // Help pop-up.
            modal.style.display = "none";
        }
    }
    
    showStatusMessageBriefly("Ready", 2000)

    // TEMPORARY: a test item for static testing.
    ///addDevice("10.0.0.27", "led-strip", "Kitchen Ambient");
    ///addDevice("10.0.0.28", "led-strip", "Kitchen Worktop");
}

function initHelp(helpId)
{
    var el = document.getElementById(helpId);

    if (el) {
        el.style.visibility = "collapse";
    }
}

function showHelp(helpId)
{
    var el = document.getElementById(helpId);

    if (el) {
        /***/
        el.style.visibility = "collapse";
        el.style.display = "none";
        /***/
        var content = el.innerHTML;
        showModal(content);
    }
}

function isOverElementOfClass(className)
{
    let x = event.clientX;
    let y = event.clientY;

    let el = document.elementFromPoint(x, y);

    return isOfClass(el, className);
}

function isOfClass(el, className)
{
    let is = false;

    if (el)
    {
        let classes = el.classList;

        let n = classes.length;
        let i = 0;

        while (!is && (i < n)) {
            if (classes[i] == className) {
                is = true;
            }
            i++;
        }
    }

    return is;
}

/* modal pop-up */

var modal;
var modalCloseBtn;
var modalText;

function showModal(text) {
    //modalText.textContent = text;
    modalText.innerHTML = text;
    modal.style.display = "block";
}

function hideModal() {
    modal.style.display = "none";
}

function makeGauge(gaugeId, minVal, maxVal, val, gaugeLabel, gaugeSymbol)
{
    let gauge = new JustGage({
        id: gaugeId,
        /* title: 'Temperature', */
        label: gaugeLabel,
        value: val,
        min: minVal,
        max: maxVal,
        /* valueFontColor: gaugeValueColor, */
        valueFontColor: '#ffffff',
        symbol: gaugeSymbol,
        pointer: true,
        gaugeWidthScale: 0.6,
        pointerOptions: {
          toplength: -10,
          bottomlength: 8,
          bottomwidth: 8,
          /*
          color: '#2181cf',
          stroke: '#ffffff',
          */
          color: '#2181cf',
          stroke: 'white',
          stroke_width: 2,
          stroke_linecap: 'round'
        },
        customSectors: [{
          color: '#022cd3',
          lo: 0,
          hi: 15
        }, {
          color: '#37c1fc',
          lo: 15,
          hi: 20
        }, {
          color: '#e2e226',
          lo: 20,
          hi: 25
        }, {
          color: '#f7ac2c',
          lo: 25,
          hi: 30
        }, {
          color: '#f7472c',
          lo: 30,
          hi: 50
        }],
        /* labelMinFontSize: 10, */
        valueMinFontSize: 25,
        counter: true,
        relativeGaugeSize: true
      });

      return gauge;
}

var iaqGauge;
var pm25Gauge;
var pm10Gauge;
var temperatureGauge;
var humidityGauge;
var tvocGauge;
var eCo2Gauge;

function makeGauges()
{
    console.log("Making gauges...");

    iaqGauge = makeIAQGauge();
    airPressureGauge = makeAirPressureGauge();
    tvocGauge = makeTVOCGauge();
    co2Gauge = makeCO2Gauge();
    temperatureGauge = makeTemperatureGauge();
    humidityGauge = makeHumidityGauge();
}

function recreateGauges() {
    /*
    iaqGauge.destroy();
    airPressureGauge.destroy();
    tvocGauge.destroy();
    co2Gauge.destroy();
    temperatureGauge.destroy();
    humidityGauge.destroy();
    */

    /*
   iaqGauge = null;
   airPressureGauge = null;
   tvocGauge = null;
   co2Gauge = null;
   temperatureGauge = null;
   humidityGauge = null;
   */

   let newIAQGauge = makeIAQGauge();
   replaceGauge('iaq-gauge', newIAQGauge);
   iaqGauge = newIAQGauge;

   let newAirPressureGauge = makeAirPressureGauge();
   replaceGauge('air-pressure-gauge', newAirPressureGauge);
   airPressureGauge = newAirPressureGauge;

   let newTVOCGauge = makeTVOCGauge();
   replaceGauge('tvoc-gauge', newTVOCGauge);
   tvocGauge = newTVOCGauge;

   let newCO2Gauge = makeCO2Gauge();
   replaceGauge('co2-gauge', newCO2Gauge);
   eCo2Gauge = newCO2Gauge;

   let newTemperatureGauge = makeTemperatureGauge();
   replaceGauge('temperature-gauge', newTemperatureGauge);
   temperatureGauge = newTemperatureGauge;

   let newHumidityGauge = makeHumidityGauge();
   replaceGauge('humidity-gauge', newHumidityGauge);
   humidityGauge = newHumidityGauge;

    window.dispatchEvent(new Event('resise'));
}

function replaceGauge(id, newGauge)
{
    let containerId = id + '-box';
    let el = document.getElementById(containerId);

    if (el) {
        let parentEl = el.parentNode;
        let id = '';
        if (parentEl.firstChild) {
            id = parentEl.firstChild.id;
             parentEl.removeChild(parentEl.firstChild);
        }
        newElement = document.createElement('div');
        newElement.id = id;
        newElement.classList.add('gauge');
        parentEl.appendChild(newElement);
        iaqGauge = null;
    }
 }

function makeIAQGauge() {
    return makeGauge('iaq-gauge', 0, 500, 0,"Air Quality Index", '');
}

function makeAirPressureGauge() {
    return makeGauge('air-pressure-gauge', 0, 2000, 0, 'Air Pressure (hPa)', '');
}

function makeTVOCGauge() {
    return makeGauge('tvoc-gauge', 0, 200, 0, 'TVOC (ppm)', '');
}

function makeCO2Gauge() {
    return makeGauge('eco2-gauge', 0, 10000, 0, 'eCO2 (ppm)', '');
}

function makeTemperatureGauge() {
    return makeGauge('temperature-gauge', 0, 50, 0, 'Temperature', '°C');
}

function makeHumidityGauge() {
    return makeGauge('humidity-gauge', 0, 100, 0, 'Relative Humidity', '');
}

  /* end modal */


/*
let themeEl =  document.getElementById('theme');
if (themeEl) {
    console.log("Setting up listening for load of theme.");
    theme.addEventListener('load', function() {
        console.log("Load theme triggered.");
    });
}
*/

window.addEventListener('load', onLoad);

document.addEventListener("DOMContentLoaded", function(event) {

makeGauges();
    
    /*
    let timer = setInterval(() => {
        let iaq = getRandomInt(0, 500);
        iaqGauge.refresh(getRandomInt(0, 500));
        iaqGauge.refresh(iaq);
        updateIAQSummary(iaq);    
    }, 5000);
    */

    /* 
    
    Used to offline test gauge animations...

    let timer = setInterval(() => {
        let n = getRandomNumber(1, 5);
        switch (n) {
            case 0:
                iaqGauge.refresh(getRandomNumber(0, 500));
                break;
            case 1:
                temperatureGauge.refresh(getRandomNumber(0, 50));
                break;
            case 2:
                humidityGauge.refresh(getRandomNumber(0, 100));
                break;
            case 3:
                tvocGauge.refresh(getRandomNumber(0, 150));
                break;
            case 4:
                eCo2Gauge.refresh(getRandomNumber(0, 10000));
                break;                                 
            case 5:
                airPressureGauge.refresh(getRandomNumber(0, 2000));
                break;                                 
                }
      }, 1000);

    */

    function getRandomNumber(min, max) {
        let n = Math.floor(Math.random() * (max - min + 1)) + min;
        return n;
    }

    let fanRotationAngle = 1;

    let fanAnimateTimer = setInterval(() => 
        {
            let el = document.getElementById("spinning-fan");

            if (el)
            {
                let newFanRotationAngle = 0;

                if (isOn()) {
                    newFanRotationAngle = fanRotationAngle + 3.0 * fanSpeed;
                    newFanRotationAngle = newFanRotationAngle % 360;
                }
                
                if (newFanRotationAngle != fanRotationAngle) {
                    fanRotationAngle = newFanRotationAngle;
                    el.style.transform = `rotate(${fanRotationAngle}deg)`;
                }
            }
        }
        , 40  /* 25Hz */
    );
    
    /*
    document.getElementById('gauge_refresh').addEventListener('click', function() {
      iaqGauge.refresh(getRandomInt(0, 500));
      temperatureGauge.refresh(getRandomInt(0, 50));
      humidityGauge.refresh(getRandomInt(0, 100));
      tvocGauge.refresh(getRandomInt(0, 150));
      eCo2Gauge.refresh(getRandomInt(0, 10000));
    });
    */
  });

