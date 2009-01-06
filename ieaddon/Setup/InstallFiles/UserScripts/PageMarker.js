// ==UserScript==
// @name          Page Marker
// @namespace     http://some.site.com
// @description   puts small red marker to every page (frame)
// @include       *
// ==/UserScript==

//debugger;

//debug

var style = "\
font-family: arial;\
font-size: 8px;\
position:absolute;\
background-color: #ff0000;\
left:0px;\
top:0px;\
width:16px;\
height:16px;\
color:ffffff;";

//document.body.insertAdjacentHTML("beforeEnd", "<div style=\""+style+"\">GD</div>");
document.body.insertAdjacentHTML("afterBegin", "<div style=\""+style+"\">GD</div>");
