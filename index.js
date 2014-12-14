
// to complie in debug mode, use
// node-gyp configure --debug 
// and select the module in the debug dir:
// var gz = require('./build/Debug/gazebo');


var fs = require('fs');
var util = require('util');

var mode = 'Release';

if (process.env['MGJS_DEBUG']){
  console.log('extrudo in DEBUG mode');
  mode = 'Debug';
}

var extrudo = require('./build/'+ mode + '/extrudo_cc');

// the options determine how it is configured
function Slicer (options) {
    this.reset(options);
}

exports.Slicer = Slicer;
 

Slicer.prototype.reset = function(options) {

   this.options = {
   "regioner" :
   {
        "fill" : 0.2
   },
   "slicer" : 
   {
       //   "angle" :1.570796326794897, // 2.356194525, //  1.570796326794897, 0.7853981633974485, 1.570796326794897,
      "nbOfShells":2,
      "firstLayerZ" : 0.33,
      "layerH" : 0.27,
      "layerW" : 0.43,
	  "infillDensity" : 0.1,  // unit: ratio to solid
      "infillShrinkingMultiplier" : 0.25,  // unit: layerW
      "insetDistanceMultiplier" : 0.9,  // unit: layerW
      "insetCuttOffMultiplier" : 0.01,  // remove short segments during insetting unit: layerW
      "writeDebugScadFiles" : false,
      "roofLayerCount" : 5,  // nb of extra layers for roofs 
      "floorLayerCount" : 5 // nb of extra layers for floor
   },
   
   "programName" : "Miracle-Grue",
   "firmware" : "v0.0",
   "machineName" : "Dub dub dub", 
   "versionStr" : "v0.0.1.0",   
   "gantry" : {
      "xyMaxHoming" : true, // false for tom, true for replicator
      "zMaxHoming" :  false, // true for tom, false for replicator
      "scalingFactor" : 1,
      "rapidMoveFeedRateXY": 6000,
      "rapidMoveFeedRateZ": 1400,
      "homingFeedRateZ" : 100
   },
   "outline" : {  // a rectangle around the part on the build platform
      "enabled"  : false,
      "distance" : 1.0
   },
   "extruders" : [
      {
         "coordinateSystemOffsetX" : 0.0,
         "extrusionTemperature" : 220.0,
         "nozzleZ" : 0,
         "zFeedRate" : 500,
         "firstLayerExtrusionProfile": "absSticky",
         "insetsExtrusionProfile" :  "absSlow",
         "infillsExtrusionProfile" : "absFast",
		 "extrusionMode" : "volumetric", // "rpm"
	  	 "feedDiameter" : 1.82 //diameter in mm of feedstock
       }
   ],
   "extrusionProfiles": {
        "absSlow": {
            "feedrate": 4800,

			//for volumetric extrusion
			"retractDistance" : 0.5, // mm
			"retractRate" : 798.0,
			"restartExtraDistance" : 0.0, // mm
    	    "extrudedDimensionsRatio" : 1.6, // ratio of width over height of extruded filament

            //for rpm extrusion
			"flow" : 2.1,
            "leadIn": 0.0,
			"leadOut": 0.0,
            "snortFlow": 35,
            "snortFeedrate": 1000, // 2400,
            "squirtFlow": 35,
            "squirtFeedrate": 1000},
        "absFast": {
            "feedrate": 4800,

			//for volumetric extrusion
			"retractDistance" : 0.5, // mm
			"retractRate" : 798.0,
			"restartExtraDistance" : 0.0, // mm
    	    "extrudedDimensionsRatio" : 1.6, // ratio of width over height of extruded filament

            //for rpm extrusion
            "flow" : 2.8,
            "leadIn": 0.0,
            "leadOut": 0.0,
            "snortFlow": 35,
            "snortFeedrate": 1000, 
            "squirtFlow": 35,
            "squirtFeedrate": 1000},
        "absSticky": {
            "feedrate": 1800,

			//for volumetric extrusion
			"retractDistance" : 0.5, // mm
			"retractRate" : 798.0,
			"restartExtraDistance" : 0.0, // mm
    	    "extrudedDimensionsRatio" : 1.6, // ratio of width over height of extruded filament

            //for rpm extrusion
            "flow" : 0.7,
            "leadIn": 0.0,
            "leadOut": 0.0,
            "snortFlow": 35,
            "snortFeedrate": 1000, 
            "squirtFlow": 35,
            "squirtFeedrate": 1000}
    },
   "platform" : {
      "automated" : false,
      "temperature" : 100.0,
      "waitingPositionX" :  52.0,
      "waitingPositionY" : -57.0,
      "waitingPositionZ" :  1.0
   },
   "gcoder" : 
   { 
      "header" : "start_replicator_dual.gcode",
      "footer" : "end_replicator_dual.gcode",
      "outline": false,  // when true, extrude along the perimeter (this should not be enabled normally)
      "insets" : true,   // when true, extrude along insets (a.k.a extrude the shells)
      "infills": true,   // when true, extrude infills paths 
      "infillFirst" : false,  // do infill before insets
      "dualtrick" : false, // use dual extrusion to color each outline
   },
   }

    if(options) {

	console.log('TODO!');
    }
   this.slicer = new extrudo.Slicer();
}

Slicer.prototype.slice= function(model, cb)
{

    var configStr = JSON.stringify(this.options);
    this.slicer.op0(configStr, model, cb);
}

