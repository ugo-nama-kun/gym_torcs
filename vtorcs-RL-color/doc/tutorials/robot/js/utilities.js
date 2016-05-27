/* exchange image inr through iobject */
function imgchange(inr, iobjekt)
{
    window.document.images[inr].src = iobjekt.src;
}

/* change two frames at once */
function changetwoframes(uri1, dest1, uri2, dest2)
{
  var frame1 = eval("parent." + dest1);
  var frame2 = eval("parent." + dest2);
  frame1.location.href = uri1;
  frame2.location.href = uri2;
}

