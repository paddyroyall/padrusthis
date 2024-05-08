function MM_preloadImages() { //v3.0
  var d = document;
  if (d.images) {
    if (!d.MM_p) d.MM_p = new Array();
    var i, j = d.MM_p.length,
      a = MM_preloadImages.arguments;
    for (i = 0; i < a.length; i++)
      if (a[i].indexOf("#") != 0) {
        d.MM_p[j] = new Image;
        d.MM_p[j++].src = a[i];
      }
  }
}

function MM_findObj(n, d) { //v4.01
  var p, i, x;
  if (!d) d = document;
  if ((p = n.indexOf("?")) > 0 && parent.frames.length) {
    d = parent.frames[n.substring(p + 1)].document;
    n = n.substring(0, p);
  }
  if (!(x = d[n]) && d.all) x = d.all[n];
  for (i = 0; !x && i < d.forms.length; i++) x = d.forms[i][n];
  for (i = 0; !x && d.layers && i < d.layers.length; i++) x = MM_findObj(n, d.layers[i].document);
  if (!x && d.getElementById) x = d.getElementById(n);
  return x;
}

function MM_swapImage() { //v3.0
  var i, j = 0,
    x, a = MM_swapImage.arguments;
  document.MM_sr = new Array;
  for (i = 0; i < (a.length - 2); i += 3)
    if ((x = MM_findObj(a[i])) != null) {
      document.MM_sr[j++] = x;
      if (!x.oSrc) x.oSrc = x.src;
      x.src = a[i + 2];
    }
}

function MM_swapImgRestore() { //v3.0
  var i, x, a = document.MM_sr;
  for (i = 0; a && i < a.length && (x = a[i]) && x.oSrc; i++) x.src = x.oSrc;
}

function mmLoadMenus() {
  if (window.mm_menu_0115123331_0) return;
  window.mm_menu_0115123331_0 = new Menu("root", 300, 25, "Arial, Helvetica, sans-serif", 12, "tomato", "white", "white", "tomato", "left", "middle", 10, 0, 100, -3, 7, true, true, true, 0, true, true);
  mm_menu_0115123331_0.addMenuItem("Glass", "location='Work_8.html'");
  mm_menu_0115123331_0.addMenuItem("Microfluidics", "location='Work_7.html'");
  mm_menu_0115123331_0.addMenuItem("Nano Machine", "location='Work_6.html'");
  mm_menu_0115123331_0.addMenuItem("Critical", "location='Work_1.html'");
  mm_menu_0115123331_0.addMenuItem("Nucleation", "location='Work_2.html'");
  mm_menu_0115123331_0.addMenuItem("Sedimentation", "location='Work_3.html'");
  mm_menu_0115123331_0.addMenuItem("Lanes", "location='Work_4.html'");
  mm_menu_0115123331_0.addMenuItem("Protein", "location='Work_protein.html'");
  mm_menu_0115123331_0.addMenuItem("Active Colloids", "location='Work_amoeba.html'");
  mm_menu_0115123331_0.addMenuItem("Fish", "location='Work_fish.html'");
  mm_menu_0115123331_0.addMenuItem("Topological&nbsp;Cluster&nbsp;Classification", "location='Work_5.html'");
  mm_menu_0115123331_0.hideOnMouseOut = true;
  mm_menu_0115123331_0.writeMenus();
}

function MM_reloadPage(init) { //reloads the window if Nav4 resized
  if (init == true) with(navigator) {
    if ((appName == "Netscape") && (parseInt(appVersion) == 4)) {
      document.MM_pgW = innerWidth;
      document.MM_pgH = innerHeight;
      onresize = MM_reloadPage;
    }
  }
  else if (innerWidth != document.MM_pgW || innerHeight != document.MM_pgH) location.reload();
}

function MM_showHideLayers() { //v6.0
  var i, p, v, obj, args = MM_showHideLayers.arguments;
  for (i = 0; i < (args.length - 2); i += 3)
    if ((obj = MM_findObj(args[i])) != null) {
      v = args[i + 2];
      if (obj.style) {
        obj = obj.style;
        v = (v == 'show') ? 'visible' : (v == 'hide') ? 'hidden' : v;
      }
      obj.visibility = v;
    }
}

function MM_openBrWindow(theURL, winName, features) { //v2.0
  window.open(theURL, winName, features);
}

function select_entry_img(item_id){
  // make the text orange
  var url = document.getElementById(item_id)
  url.href = "#"
  for (index in url.children){
    if (url.children[index].nodeName == "IMG"){
      var img = url.children[index];
      var old_path = img.src; // all images end up with 2 are black; 1 are orange
      var new_path = old_path.replace(/2.svg/, "1.svg");
      img.src = new_path;
    }
  }
}

function select_entry(item_id){
  // make the text orange
  var url = document.getElementById(item_id)
  url.href = "#"
  url.className += " menu_item_box_selected"
}

function show_recruit_box(){
  var pos_obj = document.getElementById("positions");
  var rec_obj = document.getElementById("box_recruit")
  var rect = pos_obj.getBoundingClientRect();
  MM_showHideLayers("box_recruit", "", "show");
  var box = document.getElementById("box_recruit");
  box.style.left = rect.left;
  box.style.top = rect.bottom;
}


function hide_recruit_box(){
  var pos_obj = document.getElementById("positions");
  var rec_obj = document.getElementById("box_recruit")
  var rect = pos_obj.getBoundingClientRect();
  MM_showHideLayers("box_recruit", "", "hide");
}