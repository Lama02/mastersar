
# Un makefile (experimental) pour les documents latex avec bibtex.
.SUFFIXES :
.PHONY    : all help clean distclean 

LATEX  = latex
DVIPS  = dvips
BIBTEX = bibtex
 
TEXFILE  = $(shell ls *.tex | sed -e 's/.tex//')
BIBFILES = $(shell ls *.bib)

all: $(TEXFILE).pdf

help:
	@echo "Makefile generique pour les fichiers LaTex"
	@echo "cibles : clean, veryclean, *.dvi"

clean: 
	@rm -f *~ *.log *.aux *.toc *.ilg *.idx *.ind *.bbl *.blg *.lof *.lot *.dvi *.out *.backup \
		*/*~ */*.out */*.toc */*.aux */*.backup *.pdf *.ps *.dvi  



%.pdf : %.tex
	$(LATEX) $*.tex 
	$(LATEX) $*.tex 
	$(LATEX) $*.tex
	rm *.out *.log *.toc *~ *.aux *.blg


%.ps : %.dvi
	$(DVIPS) $*.dvi -o $*.ps
