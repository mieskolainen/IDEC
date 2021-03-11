/*  ContentFlowAddOn_DEFAULT, version 2.0 
 *  (c) 2008 - 2009 Sebastian Kutsch
 *  <http://www.jacksasylum.eu/ContentFlow/>
 *
 *  This file is distributed under the terms of the MIT license.
 *  (see http://www.jacksasylum.eu/ContentFlow/LICENSE)
 */

/*
 * This is an example file of an AddOn file and will not be used by ContentFlow.
 * All values are the default values of ContentFlow.
 *
 * To create a new AddOn follow this guideline:
 *              (replace ADDONNAME by the name of your AddOn)
 *
 * 1. rename this file to ContentFlowAddOn_ADDONNAME.js
 * 2. Change the string 'DEFAULT' in the 'new ContentFlowAddOn' line to 'ADDONNAME'
 * 3. Make the changes you like/need
 * 4. Remove all settings you do not need (or comment out for testing).
 * 5. Add 'ADDONNAME' to the load attribute of the ContentFlow script tag in your web page
 * 6. Reload your page :-)
 *
 */
new ContentFlowAddOn ('DEFAULT', {

    /* 
     * This function will be executed on creation of this object (on load of this file).
     * It's mostly intended to automatically add additional stylesheets and javascripts.
     *
     * Object helper methods and parameters:
     * scriptpath:          basepath of this AddOn (without the filename)
     * addScript(path):     adds a javascript-script tag to the head with the src set to 'path'
     *                      i.e. this.addScript(scriptpath+"MyScript.js") .
     *
     * addStylesheet(path): adds a css-stylesheet-link tag to the head with href set to
     *                      'path' i.e. this.addStylesheet(scriptpath+"MyStylesheet.css") .
     *                      If path is omittet it defaults to :
     *                      scriptpath+'ContentFlowAddOn_ADDONNAME.css'.
     *
     */
    init: function() {
    },
    
    /* 
     * This method will be executed for each ContentFlow on the page after the
     * HTML document is loaded (when the whole DOM exists). You can use it to
     * add elements automatically to the flow.
     *
     * flow:                the DOM object of the ContentFlow
     * flow.Flow:           the DOM object of the 'flow' element
     * flow.Scrollbar:      the DOM object of the 'scrollbar' element
     * flow.Slider:         the DOM object of the 'slider' element
     * flow.globalCaption:  the DOM object of the 'globalCaption' element
     *
     * You can access also all public methods of the flow by 'flow.METHOD' (see documentation).
     */
    onloadInit: function (flow) {
    },
	
    /*
     * ContentFlow configuration.
     * Will overwrite the default configuration (or configuration of previously loaded AddOns).
     * For a detailed explanation of each value take a look at the documentation.
     */
	ContentFlowConf: {
        loadingTimeout: 30000,          // milliseconds
        activeElement: 'content',       // item or content

        maxItemHeight: 0,               // 0 == auto, >0 max item height in px
        scaleFactor: 1.0,               // overall scale factor of content
        scaleFactorLandscape: 1.0,      // scale factor of landscape images ('max' := height= maxItemHeight)
        relativeItemPosition: "top center", // align top/above, bottom/below, left, right, center of position coordinate
        contentPosition: "bottom",      // align at the top, center/middle or bottom of the item

        circularFlow: true,             // should the flow wrap around at begging and end?
        visibleItems: -1,               // how man item are visible on each side (-1 := auto)
        verticalFlow: false,            // turn ContentFlow 90 degree counterclockwise
        endOpacity: 1,                  // opacity of last visible item on both sides
        startItem:  "center",           // which item should be shown on startup?
        scrollInFrom: "pre",            // from where should be scrolled in?

        flowSpeedFactor: 1.0,           // how fast should it scroll?
        flowDragFriction: 1.0,          // how hard should it be be drag the floe (0 := no dragging)
        scrollWheelSpeed: 1.0,          // how fast should the mouse wheel scroll. nagive values will revers the scroll direction (0:= deactivate mouse wheel)
        keys: {                         // key => function definition, if set to {} keys ar deactivated
            37: function () { this.moveTo('pre') }, 
            38: function () { this.moveTo('visibleNext') },
            39: function () { this.moveTo('next') },
            40: function () { this.moveTo('visiblePre') }
        },

        reflectionType: "clientside",   // client-side, server-side, none
        reflectionColor: "transparent", // none, transparent, overlay or hex RGB CSS style #RRGGBB
        reflectionHeight: 0.5,          // float (relative to original image height)
        negativeMarginOnFloat: "auto",  // auto, none or float (relative to reflectionHeight)
        reflectionServerSrc: "{URLTO}{FILENAME}_reflection.{EXT}",  // {URLTO}, {FILE}, {FILENAME}, {EXT}


        /*
         * ==================== helper and calculation methods ====================
         *
         * This section contains all user definable methods. With thees you can
         * change the behavior and the visual effects of the flow.
         * For an explanation of each method take a look at the documentation.
         *
         * BEWARE:  All methods are bond to the ContentFlow!!!
         *          This means that the keyword 'this' refers to the ContentFlow 
         *          which called the method.
         */
        
        /* ==================== actions ==================== */

        /*
         * called after the inactive item is clicked.
         */
        onclickInactiveItem : function (item) {},

        /*
         * called after the active item is clicked.
         */
        onclickActiveItem: function (item) {
            var url, target;

            if (url = item.content.getAttribute('href')) {
                target = item.content.getAttribute('target');
            }
            else if (url = item.element.getAttribute('href')) {
                target = item.element.getAttribute('target');
            }
            else if (url = item.content.getAttribute('src')) {
                target = item.content.getAttribute('target');
            }

            if (url) {
                if (target)
                    window.open(url, target).focus();
                else
                    window.location.href = url;
            }
        },
        
        /*
         * called when an item becomes inactive.
         */
        onMakeInactive: function (item) {},

        
        /*
         * called when an item becomes active.
         */
        onMakeActive: function (item) {},
        
        /*
         * called when the target item/position is reached
         */
        onReachTarget: function(item) {},

        /*
         * called when a new target is set
         */
        onMoveTo: function(item) {},

        /*
         * called if the pre-button is clicked.
         */
        onclickPreButton: function (event) {
            this.moveToIndex('pre');
            Event.stop(event);
        },
        
        /*
         * called if the next-button is clicked.
         */
        onclickNextButton: function (event) {
            this.moveToIndex('next');
            Event.stop(event);
        },
        
        /* ==================== calculations ==================== */

        /*
         * calculates the width of the step.
         */
        calcStepWidth: function(diff, absDiff) {
            if (absDiff > this._visibleItems) {
                if (diff > 0) {
                    var stepwidth = diff - this._visibleItems;
                } else {
                    var stepwidth = diff + this._visibleItems;
                }
            } else if (this._visibleItems >= this.items.length) {
                var stepwidth = diff / this.items.length;
            } else {
                var stepwidth = diff * ( this._visibleItems / this.items.length);
            }
            return stepwidth;
        },

        /*
         * calculates the size of the item at its relative position x
         *
         * relativePosition: Math.round(Position(activeItem)) - Position(item)
         * side: -1, 0, 1 :: Position(item)/Math.abs(Position(item)) or 0 
         * returns a size object
         */
        calcSize: function (relativePosition, side) {
            var rP = relativePosition;
            var vI = this._visibleItems;
            var maxHeight = this.maxHeight;

            var h = maxHeight/(Math.abs(rP)+1);
            var w = h;
            return {width: w, height: h};

        },

        /*
         * calculates the position of an item within the flow depending on it's relative position
         *
         * relativePosition: Math.round(Position(activeItem)) - Position(item)
         * side: -1, 0, 1 :: Position(item)/Math.abs(Position(item)) or 0 
         */
        calcCoordinates: function (relativePosition, side) {
            var rP = relativePosition;
            var vI = this._visibleItems;
            var maxHeight = this.maxHeight;

            var f = 1 - 1/Math.exp( Math.abs(rP)*0.75);
            var x =  this.Flow.center.x * (1 + side *vI/(vI+1)* f); 
            var y = this.maxHeight;

            return {x: x, y: y};
        },
        
        /*
         * calculates the position of an item relative to it's calculated coordinates
         * x,y = 0 ==> upper left corner of item has the position calculated by
         * calculateCoordinates
         *
         * relativePosition: Math.round(Position(activeItem)) - Position(item)
         * side: -1, 0, 1 :: Position(item)/Math.abs(Position(item)) or 0 
         * size: size object calculated by calcSize
         */
        calcRelativeItemPosition: function(relativePosition, side, size) {
            var rP = relativePosition;
            var vI = this._visibleItems;
            var maxHeight = this.maxHeight;

            var x = -size.width/2;
            var y = -size.height;
            return {x: x, y: y};
        },

        /*
         * calculates and returns the relative z-index of an item
         */
        calcZIndex: function (x, f, I) {
            return -Math.abs(I);
        },

        /*
         * calculates and returns the relative font-size of an item
         */
        calcFontSize: function (x, f, size) {
            return size.height / this.maxHeight;
        },

        /*
         * calculates and returns the opacity of an item
         */
        calcOpacity: function (relativePosition, side) {
            return 1 - ((1 - this._endOpacity ) * Math.sqrt(Math.abs(relativePosition)/this._visibleItems));
        }
	
    }

});
