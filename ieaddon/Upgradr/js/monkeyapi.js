// ==UserScript==
// @name          Monkey API
// @guid          {11EF79F1-210A-16A1-0D8A-11CDFDDD}
// @description   GreaseMonkey compatibility layer
// ==/UserScript==

window.GM_Log = function(message) 
{ 
	return UpgradrShell.Log(message); 
}

window.GM_SetValue = function(key, value) 
{ 
	return UpgradrShell.SetValue(key, value); 
}

window.GM_GetValue = function(key, defaultValue) 
{ 
  if (!UpgradrShell.HasValue(key)) return defaultValue;
  return UpgradrShell.GetValue(key);
}