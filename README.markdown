# Upgradr - once aimed to be a "GreaseMonkey for IE"

Well, this is my unfinished project. IE addon which wanted to be a [GreaseMonkey][greasemonkey] for IE.

If you are looking for something similar, give [IE7Pro][ie7pro] a try.

Feel free to pick bits of this if you want to dig in IE internals and kinky COM stuff. But beware of ATL/WTL fever and ActiveScript madness.

## Features one may find interesting:

* **IE Tabs/Frames manager** = it tracks a tree of IHTMLDocument2 for open tabs and their frames - insanely hard to do correctly and keep it consistent during browser refreshes
* **Scripts manager** = enables to inject javascript into any frame and makes it possible to debug these in Visual Studio - yup, pretty hard to do
* **Some GUI tricks** = showing WTL/ATL/COM and GUI threading tricks under IE

## R.I.P. 2007

[greasemonkey]:http://en.wikipedia.org/wiki/Greasemonkey
[ie7pro]:http://en.wikipedia.org/wiki/IE7pro