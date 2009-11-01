CMDLATEX = pdflatex
OUTPUT_DIR = genereted_pdf
SRC= $(wildcard *.tex) 
PDF= $(SRC:.tex=.pdf) 

all : help

help :
	@(echo)
	@(echo Usage :)
	@(echo make pdf       : generate pdfs)
	@(echo make clean     : remove all generated files except pdf files)
	@(echo make clean_all : remove all generated files)
	@(echo)

pdf : $(PDF)

%.pdf : %.tex
	$(CMDLATEX) $^ 
	$(CMDLATEX) $^ 
clean :
	rm -fr *.aux *.out *.log *.toc *~
	-cd templates && rm -fr *.aux *.out *.log *.toc *~
clean_all : clean
	rm *.pdf

