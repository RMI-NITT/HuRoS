body *").on("mousedown.feedback",
hj.tryCatch(function(b){b.stopPropagation();b.preventDefault();e(b.target)&&(a(b.target),d(b.target,hj.hq(this)))},"feedback"))},0)},"feedback");b.cancelElementHighlighting=hj.tryCatch(function(){hj.hq("body *").off("mouseover.feedback mousedown.feedback");hj.hq(document).off("scroll.feedback resize.feedback");hj.hq("html").removeAttr("data-hotjar-cursor-pointer")},"feedback");b.resetHighlight=hj.tryCatch(function(){var a=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_top"),
b=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_right"),c=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_bottom"),d=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_left"),e=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_highlighter");hj.hq(a).removeAttr("style");hj.hq(b).removeAttr("style");hj.hq(c).removeAttr("style");hj.hq(d).removeAttr("style");hj.hq(e).removeAttr("style")});
b.highlightElement=hj.tryCatch(function(a){var b=a.getBoundingClientRect();a=b.top;var c=b.left,d=b.width,b=b.height,e=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_top"),f=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_right"),h=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_bottom"),g=document.getElementById(hj.widget.widgetAttributePrefix+"_feedback_content_dimmer_left"),l=document.getElementById(hj.widget.widgetAttributePrefix+
"_feedback_content_highlighter"),m=8,n=a-m,r=c-8,B=b+2*m,C=d+16;0>a&&(B=b+a+m,m=0,b+=a,n=a=1);e.style.top="0";e.style.right="0";e.style.left="0";e.style.height=Math.max(0,n)+"px";f.style.top=n+"px";f.style.right="0";f.style.left=r+C+"px";f.style.bottom="0";h.style.top=n+B+"px";h.style.width=Math.max(0,C)+"px";h.style.left=r+"px";h.style.bottom="0";g.style.top=n+"px";g.style.width=Math.max(0,r