// ==UserScript==
// @name          Event Handler Example
// @namespace     http://some.site.com
// @description   example of using event handler
// @include       *
// ==/UserScript==

function click_event()
{
	alert("clicked!");
}

var el = document.getElementById("clickable-button");
if (el)
{
	el.onclick = click_event;
}