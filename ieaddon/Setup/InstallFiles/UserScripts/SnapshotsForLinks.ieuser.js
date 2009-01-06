// ==UserScript==
// @name		Snap Shots
// @namespace		http://iescripts.org/
// @description		Add snap.com effect to sites
// @include		*
// ==/UserScript==

if (typeof window.SNAP_COM == "undefined") { window.SNAP_COM = {}; } window.SNAP_COM.shot_config = {version:"2.1.0",prefix:{image:"http://shots.snap.com/images/v2.1.0/",image_sized:"http://shots.snap.com/images/v2.1.0/size_305/",theme:"http://shots.snap.com/images/v2.1.0/theme/silver/",link:"http://shots.snap.com/",options:"http://shots.snap.com/",preview:"http://shots.snap.com/",spasense:"http://shots.snap.com/",bg:"http://shots.snap.com/images/v2.1.0/theme/silver/bg/bg_320x317",snap:"http://www.snap.com/",search:"http://shots.snap.com/"},size:"small",theme:"silver",key:"",source:"",flavor:null,lang:"en-us",force:null,check_defer:false,observe_event:"body",search_box:false,auto_preview:false,show_internal:false,preview_only:false,platform:false,show_link_icon:false,preview_trigger:"both",search_type:"spasense",redirect_param:null,delay:{show:500,hide:150,move:200},img:{cobrand:"http://shots.snap.com/images/v2.1.0/t.gif",submit:"http://shots.snap.com/images/v2.1.0/t.gif",link_icon:"http://shots.snap.com/images/v2.1.0/t.gif",palette:"http://shots.snap.com/images/v2.1.0/theme/silver/en-us/palette.gif"},css_pos:{main:{x:0,y:0,w:320,h:317},pointer:{x:0,y:0,w:50,h:38},bubble:{x:11,y:38,w:298,h:239},bubble_img:{x:0,y:0,w:298,h:239},body:{x:0,y:0,w:298,h:239},bg_img:{x:0,y:0,w:320,h:317},top_right_menu:{xr:16,y:3,w:null,h:13},pin_close_img:{xr:8,y:-2,w:null,h:null},preview_div:{x:7,y:7,w:282,h:200},preview:{x:0,y:0,w:null,h:null},loading_img:{x:0,y:0,w:null,h:null},search:{x:8,yb:26,w:284,h:30},logo_a:{xr:7,yb:8,w:null,h:null},box:{x:0,y:5,w:233,h:20},submit:{xr:0,y:5,w:46,h:20},cobrand_img:{x:9,y:213,w:null,h:null},cobrand_a:{x:10,yb:8,w:null,h:15},options:{x:4,y:0,w:298,h:null},option_a:{xr:12,y:3,w:null,h:null},option_iframe:{x:4,y:7,w:282,h:200},spasense_iframe:{x:8,yb:26,w:282,h:32},link_icon:{x:null,y:null,w:14,h:12}},style:{link_icon:{padding:"1px 0 0 0",textDecoration:"none",position:"static"},option_a:{fontSize:"10px",fontWeight:"normal",color:"#857a7a",textDecoration:"none"},preview_div:{border:"1px solid #c4c4c4",overflow:"hidden",backgroundColor:"#FFFFFF"},search:{},box:{fontSize:"10px",border:"1px solid #999999",color:"#333333",backgroundColor:"#f3f3f3"}},offset:{search_box:30},hidden:["spasense_iframe","link_icon","search"],t_img:["cobrand_img","bg_img"],partial_check:{delay:2000,attempts:5},href:{logo:"http://www.snap.com/",cobrand:"http://www.snap.com/about/shots.php"},palette:{submit:{loc:"-271px 0",w:46,h:20},cobrand_a:{loc:"-333px 0",w:92,h:15},link_icon:{loc:"-519px 0",w:14,h:12},logo_a:{loc:"-425px 0",w:81,h:15},previewby:{loc:"-164px 0",w:107,h:21},ques_img:{loc:"-723px 0",w:48,h:11},close_img:{loc:"-865px 0",w:46,h:11},option_a:{loc:"-533px 0",w:95,h:12}},pointer:{tl:{x:0,y:0},tr:{x:270,y:0},bl:{x:0,y:279},br:{x:270,y:279}},text:{Options:"Options",Disable:"Disable",SearchTheWeb:"Search the Web on Snap.com",GoToURL:"Go to %URL",SubmitSearch:"Submit your search",SignUpLink:"Sign Up to add Free Snap Shots to your site!",SnapLogoTooltip:"Powered by Snap",OptionsTooltip:"Snap Shots Options",OptionsClose:"Close Options"},color:{bubble:"#dcdcdc",bubble_rollover:"#dcdcdc",bubble_border:"#a4a3a3",url:"#003399",url_down:"#ff0000",preview_border:"#c4c4c4",preview_border_down:"#ff0000",options:"#857a7a",options_down:"#ff0000",search_box:"",search_box_border:"#dcdcdc",search_field:"#f3f3f3",search_field_border:"#999999"},fl:""};

window.SNAP_COM.shot_main_js = function() {
  var s = document.getElementsByTagName("script")[0];
  if (s==null) return;
  var js = document.createElement("script");
  js.type = "text/javascript";
  js.src = "http://shots.snap.com/shot_main_js/v2.1.0/";
  s.parentNode.insertBefore(js, s);
}
window.SNAP_COM.window_onload = function() {
  window.SNAP_COM.window_loaded = true;
};
if (window.addEventListener) {
  window.addEventListener( "load", window.SNAP_COM.window_onload, false );
} else if (window.attachEvent) {
  window.attachEvent( "onload", window.SNAP_COM.window_onload );
}

var elms = document.getElementsByTagName('a');
for(var i=0; i<elms.length; i++){
	var cname = elms[i].className;
	if(cname.indexOf('snap_shots') != -1) continue;
	if(cname != '') cname += ' snap_shots';
	else cname = 'snap_shots';

	elms[i].className = cname;
}

window.SNAP_COM.shot_main_js();
