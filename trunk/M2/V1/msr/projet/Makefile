Rapport-etudiants.pdf: *.tex *.eps
	latex Rapport-etudiants.tex
	latex Rapport-etudiants.tex
	dvips -f Rapport-etudiants.dvi > Rapport-etudiants.ps
	ps2pdf Rapport-etudiants.ps
	rm Rapport-etudiants.ps

clean:
	rm -f *.aux *.dvi *.log *.idx *.ilg *.toc *.ind *.out *.blg *.bbl

realclean:
	make clean
	rm -f Rapport-etudiants.pdf
