.SUFFIXES: .texi .html .info .dvi

.texi.html:
	texi2html $<

.texi.info:
	makeinfo $<

.texi.dvi:
	texinfo $<
        
DOC = solid2

default: $(DOC).info $(DOC).dvi $(DOC).html 

clean: 
	-rm *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr

realclean: clean
	-rm *.dvi *.info *.html
