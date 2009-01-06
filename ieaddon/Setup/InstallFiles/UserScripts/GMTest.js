// ==UserScript==
// @name          GreaseMonkeyAPI test
// @namespace     http://some.site.com
// @description   get/set value test
// @include       *
// ==/UserScript==

// this script adds small clickable SET / GET blue buttons on top of page
// SET stores script value
// GET retrives script value
//
// see http://diveintogreasemonkey.org/api/index.html

var style = "\
font-family: arial;\
font-size: 8px;\
position:absolute;\
background-color: blue;\
top:0px;\
padding-left:2px;\
padding-right:2px;\
cursor:hand;\
color:ffffff;";

// this works because i've put test_get function into window object, which is default scope for events fired on element
window.test_get = function()
{
	var key = prompt("Which Key?", "");
	var val = GM_GetValue(key);
  alert(val);
}
document.body.insertAdjacentHTML("afterBegin", "<div onclick=\"test_get();\" style=\""+style+"\;left:120px;\">GET</div>");


// this technique doesn't expose function in global scope
// but we need to setup event link directly
function test_set()
{
	var key = prompt("Key?", "");
	var val = prompt("Value?", "");
  GM_SetValue(key, val);
}

// THIS WON'T WORK: document.body.insertAdjacentHTML("afterBegin", "<div onclick=\"test_set();\" style=\""+style+"\;left:100px;\">SET</div>");
document.body.insertAdjacentHTML("afterBegin", "<div id=\"testset\" style=\""+style+"\;left:100px;\">SET</div>");
var el = document.getElementById("testset");
if (el)
{
	el.onclick = test_set;
}