 # Jason Ye - yeja@oregonstate.edu
 # Corey Hemphill - hemphilc@oregonstate.edu
 # CS444 - Project 2 Report Makefile
 # October 23, 2017
 # Makefile

#************************************************************************
# Reference: makefile from Materials from Class by Dr. Kevin McGrath
#************************************************************************

LATEX   = latex -shell-escape
DVIPS   = dvips
DVIPDF  = dvipdft
XDVI    = xdvi -gamma 4
GH      = gv

EXAMPLES = $(wildcard *.h)
SRC     := $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG     = $(SRC:%.tex=%.dvi)
PSF     = $(SRC:%.tex=%.ps)
PDF     = $(SRC:%.tex=%.pdf)

default: all

all: pdf

pdf: $(PDF)

ps: $(PSF)

$(TRG): %.dvi: %.tex $(EXAMPLES)
        $(LATEX) $<
        $(LATEX) $<
        $(LATEX) $<

$(PSF):%.ps: %.dvi
        $(DVIPS) -R -Poutline -t letter $< -o $@

$(PDF): %.pdf: %.ps
        ps2pdf $<

show: $(TRG)
        @for i in $(TRG) ; do $(XDVI) $$i & done

showps: $(PSF)
        @for i in $(PSF) ; do $(GH) $$i & done

clean:
        rm -f *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg

.PHONY: all show clean ps pdf showps
